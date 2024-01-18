


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <fcntl.h>


// 这些宏用于内存对齐，保证分配的内存地址满足某些对齐要求。例如，MP_ALIGNMENT 定义了32字节对齐。
#define MP_ALIGNMENT       		32
#define MP_PAGE_SIZE			4096
#define MP_MAX_ALLOC_FROM_POOL	(MP_PAGE_SIZE-1)

#define mp_align(n, alignment) (((n)+(alignment-1)) & ~(alignment-1))
#define mp_align_ptr(p, alignment) (void *)((((size_t)p)+(alignment-1)) & ~(alignment-1))




// 用来管理大块内存分配的结构体。
struct mp_large_s {
	struct mp_large_s *next;
	void *alloc;
};
// 代表内存池中的一个固定大小的内存块，包含指向内存块末尾的指针 last、指向内存块实际结束位置的指针 end、下一个内存块的指针 next，以及失败次数记录 failed。
struct mp_node_s {

	unsigned char *last;
	unsigned char *end;
	
	struct mp_node_s *next;
	size_t failed;
};
// 代表整个内存池，包含最大内存分配限制 max、当前操作的内存块 current、大内存块链表 large，和一个头部内存块 head。
struct mp_pool_s {

	size_t max;

	struct mp_node_s *current;
	struct mp_large_s *large;

	struct mp_node_s head[0];

};
// 内存池创建和销毁
struct mp_pool_s *mp_create_pool(size_t size);
void mp_destory_pool(struct mp_pool_s *pool);
// 内存分配和释放函数
void *mp_alloc(struct mp_pool_s *pool, size_t size); //mp_alloc 从内存池中分配内存，先尝试从固定大小的内存块分配，如果请求的大小太大，将使用 mp_alloc_large 从系统中分配大内存块。
void *mp_nalloc(struct mp_pool_s *pool, size_t size); // mp_nalloc 类似于 mp_alloc，但不进行内存对齐。
void *mp_calloc(struct mp_pool_s *pool, size_t size); // mp_calloc 分配内存并初始化为0。
void mp_free(struct mp_pool_s *pool, void *p); // mp_free 释放之前分配的大块内存。


struct mp_pool_s *mp_create_pool(size_t size) {

	struct mp_pool_s *p;
	int ret = posix_memalign((void **)&p, MP_ALIGNMENT, size + sizeof(struct mp_pool_s) + sizeof(struct mp_node_s));
	if (ret) {
		return NULL;
	}
	
	p->max = (size < MP_MAX_ALLOC_FROM_POOL) ? size : MP_MAX_ALLOC_FROM_POOL;
	p->current = p->head;
	p->large = NULL;

	p->head->last = (unsigned char *)p + sizeof(struct mp_pool_s) + sizeof(struct mp_node_s);
	p->head->end = p->head->last + size;

	p->head->failed = 0;

	return p;

}

void mp_destory_pool(struct mp_pool_s *pool) {

	struct mp_node_s *h, *n;
	struct mp_large_s *l;

	for (l = pool->large; l; l = l->next) {
		if (l->alloc) {
			free(l->alloc);
		}
	}

	h = pool->head->next;

	while (h) {
		n = h->next;
		free(h);
		h = n;
	}

	free(pool);

}

void mp_reset_pool(struct mp_pool_s *pool) {

	struct mp_node_s *h;
	struct mp_large_s *l;

	for (l = pool->large; l; l = l->next) {
		if (l->alloc) {
			free(l->alloc);
		}
	}

	pool->large = NULL;

	for (h = pool->head; h; h = h->next) {
		h->last = (unsigned char *)h + sizeof(struct mp_node_s);
	}

}

static void *mp_alloc_block(struct mp_pool_s *pool, size_t size) {

	unsigned char *m;
	struct mp_node_s *h = pool->head;
	size_t psize = (size_t)(h->end - (unsigned char *)h);
	
	int ret = posix_memalign((void **)&m, MP_ALIGNMENT, psize);
	if (ret) return NULL;

	struct mp_node_s *p, *new_node, *current;
	new_node = (struct mp_node_s*)m;

	new_node->end = m + psize;
	new_node->next = NULL;
	new_node->failed = 0;

	m += sizeof(struct mp_node_s);
	m = mp_align_ptr(m, MP_ALIGNMENT);
	new_node->last = m + size;

	current = pool->current;

	for (p = current; p->next; p = p->next) {
		if (p->failed++ > 4) {
			current = p->next;
		}
	}
	p->next = new_node;

	pool->current = current ? current : new_node;

	return m;

}

static void *mp_alloc_large(struct mp_pool_s *pool, size_t size) {

	void *p = malloc(size);
	if (p == NULL) return NULL;

	size_t n = 0;
	struct mp_large_s *large;
	for (large = pool->large; large; large = large->next) {
		if (large->alloc == NULL) {
			large->alloc = p;
			return p;
		}
		if (n ++ > 3) break;
	}

	large = mp_alloc(pool, sizeof(struct mp_large_s));
	if (large == NULL) {
		free(p);
		return NULL;
	}

	large->alloc = p;
	large->next = pool->large;
	pool->large = large;

	return p;
}

void *mp_memalign(struct mp_pool_s *pool, size_t size, size_t alignment) {

	void *p;
	
	int ret = posix_memalign(&p, alignment, size);
	if (ret) {
		return NULL;
	}

	struct mp_large_s *large = mp_alloc(pool, sizeof(struct mp_large_s));
	if (large == NULL) {
		free(p);
		return NULL;
	}

	large->alloc = p;
	large->next = pool->large;
	pool->large = large;

	return p;
}




void *mp_alloc(struct mp_pool_s *pool, size_t size) {

	unsigned char *m;
	struct mp_node_s *p;

	if (size <= pool->max) {

		p = pool->current;

		do {
			
			m = mp_align_ptr(p->last, MP_ALIGNMENT);
			if ((size_t)(p->end - m) >= size) {
				p->last = m + size;
				return m;
			}
			p = p->next;
		} while (p);

		return mp_alloc_block(pool, size);
	}

	return mp_alloc_large(pool, size);
	
}


void *mp_nalloc(struct mp_pool_s *pool, size_t size) {

	unsigned char *m;
	struct mp_node_s *p;

	if (size <= pool->max) {
		p = pool->current;

		do {
			m = p->last;
			if ((size_t)(p->end - m) >= size) {
				p->last = m+size;
				return m;
			}
			p = p->next;
		} while (p);

		return mp_alloc_block(pool, size);
	}

	return mp_alloc_large(pool, size);
	
}

void *mp_calloc(struct mp_pool_s *pool, size_t size) {

	void *p = mp_alloc(pool, size);
	if (p) {
		memset(p, 0, size);
	}

	return p;
	
}

void mp_free(struct mp_pool_s *pool, void *p) {

	struct mp_large_s *l;
	for (l = pool->large; l; l = l->next) {
		if (p == l->alloc) {
			free(l->alloc);
			l->alloc = NULL;

			return ;
		}
	}
	
}

/*
main 函数
这是一个测试 main 函数，它演示了如何创建内存池、从池中分配和释放内存、以及最终销毁内存池。

这个内存池管理器主要为了高效管理内存分配和释放，但是它并不管理小块内存的释放（它只能释放大块内存）。因为内存池通常在一块内存生命周期结束时一次性释放所有内存块。在分配小块内存时，内存池管理器不跟踪这些小块内存的具体情况，所以不能单独释放它们。这种设计模式在生命周期明确且内存使用量较大的场景中非常有用。
*/
int main(int argc, char *argv[]) {

	int size = 1 << 12;

	struct mp_pool_s *p = mp_create_pool(size);

	int i = 0;
	for (i = 0;i < 10;i ++) {

		void *mp = mp_alloc(p, 512);
//		mp_free(mp);
	}

	//printf("mp_create_pool: %ld\n", p->max);
	printf("mp_align(123, 32): %d, mp_align(17, 32): %d\n", mp_align(24, 32), mp_align(17, 32));
	//printf("mp_align_ptr(p->current, 32): %lx, p->current: %lx, mp_align(p->large, 32): %lx, p->large: %lx\n", mp_align_ptr(p->current, 32), p->current, mp_align_ptr(p->large, 32), p->large);

	int j = 0;
	for (i = 0;i < 5;i ++) {

		char *pp = mp_calloc(p, 32);
		for (j = 0;j < 32;j ++) {
			if (pp[j]) {
				printf("calloc wrong\n");
			}
			printf("calloc success\n");
		}
	}

	//printf("mp_reset_pool\n");

	for (i = 0;i < 5;i ++) {
		void *l = mp_alloc(p, 8192);
		mp_free(p, l);
	}

	mp_reset_pool(p);

	//printf("mp_destory_pool\n");
	for (i = 0;i < 58;i ++) {
		mp_alloc(p, 256);
	}

	mp_destory_pool(p);

	return 0;

}



