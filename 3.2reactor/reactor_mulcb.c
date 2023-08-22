#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <sys/poll.h>
#include <sys/epoll.h>


#include <pthread.h>
 
#define MAXLNE  4096

#define POLL_SIZE	1024

#define BUFFER_LENGTH		1024
#define MAX_EPOLL_EVENT		1024

#define NOSET_CB	0
#define READ_CB		1
#define WRITE_CB	2
#define ACCEPT_CB	3
/*
这份代码实现了一个多回调的Reactor模式网络服务器:

1. 定义了事件结构nitem,包含fd,事件类型,回调函数等。

2. 定义了实例结构reactor,包含epoll fd和事件块链表。

3. init_reactor初始化reactor实例,创建epoll和事件数组。

4. nreactor_set_event将fd和回调函数注册到reactor监听。

5. 定义了read_callback,write_callback,accept_callback回调。

6. 在accept_callback中处理新连接,注册read回调。

7. 在read_callback中读取数据,注册write回调。 

8. 在write_callback中写回数据,注册read回调。

9. reactor_loop是事件循环,通过epoll监听事件并调用回调。

10. 主函数中启动服务器,注册accept回调。


这种模式的特点:

- 通过epoll监听文件描述符事件
- 使用回调函数处理具体的IO操作
- 回调函数只负责一个功能,简单明了
- 通过注册回调函数改变fd监听事件

综上,这实现了一个反应器模式的网络服务器,通过回调+epoll来高效处理连接的多种状态。

比单回调 modelo 结构更清晰,可以方便实现 TCP 的各种状态切换。
*/

/*
多回调和单回调Reactor模式的主要区别在于:

1. 事件结构中的回调函数存储

- 单回调只有一个callback函数指针
- 多回调使用多个函数指针,如readcb、writecb等

2. 回调函数的定义

- 单回调的callback需要处理所有事件
- 多回调每个函数只处理一种事件

3. 事件分发

- 单回调根据事件调用同一个callback
- 多回调根据事件类型调用不同注册的回调函数

4. 回调函数内部逻辑

- 单回调需要处理各种情况,比如读写、接受等
- 多回调只需处理注册的事件类型

总结一下:

多回调通过定义多个函数指针与事件类型对应,实现了事件处理的拆分,每个回调函数只需处理一类事件。

这使得代码结构更清晰,也更方便管理每个事件的状态转移。

多回调是反应器模式里一个重要的改进,可以帮助构建模块化和可维护的网络服务器。
*/

typedef int NCALLBACK(int fd, int event, void *arg);


struct nitem { // fd

	int fd;

	int status;
	int events;
	void *arg;
#if 0
	NCALLBACK callback;
#else
	NCALLBACK *readcb;   // epollin
	NCALLBACK *writecb;  // epollout
	NCALLBACK *acceptcb; // epollin
#endif
	unsigned char sbuffer[BUFFER_LENGTH]; //
	int slength;

	unsigned char rbuffer[BUFFER_LENGTH];
	int rlength;
	
};

struct itemblock {

	struct itemblock *next;
	struct nitem *items;

};

struct reactor {

	int epfd;
	struct itemblock *head; 

};

int init_reactor(struct reactor *r);

int read_callback(int fd, int event, void *arg);

int write_callback(int fd, int event, void *arg);

int accept_callback(int fd, int event, void *arg);


struct reactor *instance = NULL;

struct reactor *getInstance(void) { //singleton

	if (instance == NULL) {

		instance = malloc(sizeof(struct reactor));
		if (instance == NULL) return NULL;
		memset(instance, 0, sizeof(struct reactor));

		if (0 > init_reactor(instance)) {
			free(instance);
			return NULL;
		}

	}

	return instance;
}



int nreactor_set_event(int fd, NCALLBACK cb, int event, void *arg) {

	struct reactor *r = getInstance();
	
	struct epoll_event ev = {0};
	
	if (event == READ_CB) {
		r->head->items[fd].fd = fd;
		r->head->items[fd].readcb = cb;
		r->head->items[fd].arg = arg;

		ev.events = EPOLLIN;
		
	} else if (event == WRITE_CB) {
		r->head->items[fd].fd = fd;
		r->head->items[fd].writecb = cb;
		r->head->items[fd].arg = arg;

		ev.events = EPOLLOUT;
	} else if (event == ACCEPT_CB) {
		r->head->items[fd].fd = fd;
		r->head->items[fd].acceptcb = cb;
		r->head->items[fd].arg = arg;

		ev.events = EPOLLIN;
	}

	ev.data.ptr = &r->head->items[fd];

	
	if (r->head->items[fd].events == NOSET_CB) {
		if (epoll_ctl(r->epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
			printf("epoll_ctl EPOLL_CTL_ADD failed, %d\n", errno);
			return -1;
		}
		r->head->items[fd].events = event;
	} else if (r->head->items[fd].events != event) {

		if (epoll_ctl(r->epfd, EPOLL_CTL_MOD, fd, &ev) < 0) {
			printf("epoll_ctl EPOLL_CTL_MOD failed\n");
			return -1;
		}
		r->head->items[fd].events = event;
	}

	
	return 0;
}

int nreactor_del_event(int fd, NCALLBACK cb, int event, void *arg) {

	struct reactor *r = getInstance();
	
	struct epoll_event ev = {0};
	ev.data.ptr = arg;

	epoll_ctl(r->epfd, EPOLL_CTL_DEL, fd, &ev);
	r->head->items[fd].events = 0;

	return 0;
}



int write_callback(int fd, int event, void *arg) {

	struct reactor *R = getInstance();
	
	unsigned char *sbuffer = R->head->items[fd].sbuffer;
	int length = R->head->items[fd].slength;

	int ret = send(fd, sbuffer, length, 0);

	if (ret < length) {
		nreactor_set_event(fd, write_callback, WRITE_CB, NULL);
	} else {
		nreactor_set_event(fd, read_callback, READ_CB, NULL);
	}
	return 0;
}

// 5k qps

int read_callback(int fd, int event, void *arg) {

	struct reactor *R = getInstance();

	unsigned char *buffer = R->head->items[fd].rbuffer;

	
#if 0 //ET
	int idx = 0, ret = 0;
	while (idx < BUFFER_LENGTH) {

		ret = recv(fd, buffer+idx, BUFFER_LENGTH-idx, 0);
		if (ret == -1) { 
			break;
		} else if (ret > 0) {
			idx += ret;
		} else {// == 0
			break;
		}

	}

	if (idx == BUFFER_LENGTH && ret != -1) {
		nreactor_set_event(fd, read_callback, READ_CB, NULL);
	} else if (ret == 0) {
		nreactor_set_event
		//close(fd);
	} else {
		nreactor_set_event(fd, write_callback, WRITE_CB, NULL);
	}
	
#else //LT

	int ret = recv(fd, buffer, BUFFER_LENGTH, 0);
	if (ret == 0) { // fin
		
		nreactor_del_event(fd, NULL, 0, NULL);
		close(fd);
		
	} else if (ret > 0) {

		unsigned char *sbuffer = R->head->items[fd].sbuffer;
		memcpy(sbuffer, buffer, ret);
		R->head->items[fd].slength = ret;

		printf("readcb: %s\n", sbuffer);
		nreactor_set_event(fd, write_callback, WRITE_CB, NULL);
	}
		
#endif

	

}


// web server 
// ET / LT
int accept_callback(int fd, int event, void *arg) {

	int connfd;
	struct sockaddr_in client;
    socklen_t len = sizeof(client);
    if ((connfd = accept(fd, (struct sockaddr *)&client, &len)) == -1) {
        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

	nreactor_set_event(connfd, read_callback, READ_CB, NULL);

}



int init_server(int port) {

	int listenfd;
    struct sockaddr_in servaddr;
    char buff[MAXLNE];
 
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
 
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    if (listen(listenfd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
	return listenfd;

}

int init_reactor(struct reactor *r) {

	if (r == NULL) return -1;

	int epfd = epoll_create(1); //int size
	r->epfd = epfd;

	// fd --> item
	r->head = (struct itemblock*)malloc(sizeof(struct itemblock));
	if (r->head == NULL) {
		close(epfd);
		return -2;
	} 
	memset(r->head, 0, sizeof(struct itemblock));

	r->head->items = malloc(MAX_EPOLL_EVENT * sizeof(struct nitem));
	if (r->head->items == NULL) {
		free(r->head);
		close(epfd);
		return -2;
	}
	memset(r->head->items, 0, (MAX_EPOLL_EVENT * sizeof(struct nitem)));
	
	r->head->next = NULL;
	
	return 0;
}

// accept --> EPOLL
int reactor_loop(int listenfd) {

	struct reactor *R = getInstance();
	
	struct epoll_event events[POLL_SIZE] = {0};
	while (1) {

		int nready = epoll_wait(R->epfd, events, POLL_SIZE, 5);
		if (nready == -1) {
			continue;
		}

		int i = 0;
		for (i = 0;i < nready;i ++) {
			
			struct nitem *item = (struct nitem *)events[i].data.ptr;
			int connfd = item->fd;

			if (connfd == listenfd) { //
				item->acceptcb(listenfd, 0, NULL);
			} else {
			
				if (events[i].events & EPOLLIN) { //
					item->readcb(connfd, 0, NULL);
				
				} 
				if (events[i].events & EPOLLOUT) {
					item->writecb(connfd, 0, NULL);
		
				}
			}
		}

	}

	return 0;
}


int main(int argc, char **argv) 
{
    
 	int  connfd, n;

	int listenfd = init_server(9999);
	nreactor_set_event(listenfd, accept_callback, ACCEPT_CB, NULL);

	//nreactor_set_event(listenfd, accept_callback, read_callback, write_callback);
	

	reactor_loop(listenfd);
	 
    return 0;
}

