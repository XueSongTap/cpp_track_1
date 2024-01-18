

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>


// 使用宏定义LL_ADD和LL_REMOVE来简化链表操作。
#define LL_ADD(item, list) do { 	\
	item->prev = NULL;				\
	item->next = list;				\
	list = item;					\
} while(0)

#define LL_REMOVE(item, list) do {						\
	if (item->prev != NULL) item->prev->next = item->next;	\
	if (item->next != NULL) item->next->prev = item->prev;	\
	if (list == item) list = item->next;					\
	item->prev = item->next = NULL;							\
} while(0)

//代码中定义了三个结构：nWorker表示线程池中的工作线程，nJob表示工作队列中的工作项，nWorkQueue表示线程池。
typedef struct NWORKER {
	pthread_t thread;
	int terminate;
	struct NWORKQUEUE *workqueue;
	struct NWORKER *prev;
	struct NWORKER *next;
} nWorker;

typedef struct NJOB {
	void (*job_function)(struct NJOB *job);
	void *user_data;
	struct NJOB *prev;
	struct NJOB *next;
} nJob;

typedef struct NWORKQUEUE {
	struct NWORKER *workers;
	struct NJOB *waiting_jobs;
	pthread_mutex_t jobs_mtx;
	pthread_cond_t jobs_cond;
} nWorkQueue;

typedef nWorkQueue nThreadPool;
//ntyWorkerThread是线程函数，每个线程在这个函数中循环，等待工作项，然后执行它们。
static void *ntyWorkerThread(void *ptr) {
	nWorker *worker = (nWorker*)ptr;

	while (1) {
		pthread_mutex_lock(&worker->workqueue->jobs_mtx);

		while (worker->workqueue->waiting_jobs == NULL) {
			if (worker->terminate) break;
			pthread_cond_wait(&worker->workqueue->jobs_cond, &worker->workqueue->jobs_mtx);
		}

		if (worker->terminate) {
			pthread_mutex_unlock(&worker->workqueue->jobs_mtx);
			break;
		}
		
		nJob *job = worker->workqueue->waiting_jobs;
		if (job != NULL) {
			LL_REMOVE(job, worker->workqueue->waiting_jobs);
		}
		
		pthread_mutex_unlock(&worker->workqueue->jobs_mtx);

		if (job == NULL) continue;

		job->job_function(job);
	}

	free(worker);
	pthread_exit(NULL);
}


//ntyThreadPoolCreate函数创建指定数量的工作线程，并将它们加入线程池。
int ntyThreadPoolCreate(nThreadPool *workqueue, int numWorkers) {

	if (numWorkers < 1) numWorkers = 1;
	memset(workqueue, 0, sizeof(nThreadPool));
	
	pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
	memcpy(&workqueue->jobs_cond, &blank_cond, sizeof(workqueue->jobs_cond));
	
	pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
	memcpy(&workqueue->jobs_mtx, &blank_mutex, sizeof(workqueue->jobs_mtx));

	int i = 0;
	for (i = 0;i < numWorkers;i ++) {
		nWorker *worker = (nWorker*)malloc(sizeof(nWorker));
		if (worker == NULL) {
			perror("malloc");
			return 1;
		}

		memset(worker, 0, sizeof(nWorker));
		worker->workqueue = workqueue;

		int ret = pthread_create(&worker->thread, NULL, ntyWorkerThread, (void *)worker);
		if (ret) {
			
			perror("pthread_create");
			free(worker);

			return 1;
		}

		LL_ADD(worker, worker->workqueue->workers);
	}

	return 0;
}

//ntyThreadPoolShutdown函数关闭线程池，停止所有工作线程。
void ntyThreadPoolShutdown(nThreadPool *workqueue) {
	nWorker *worker = NULL;

	for (worker = workqueue->workers;worker != NULL;worker = worker->next) {
		worker->terminate = 1;
	}

	pthread_mutex_lock(&workqueue->jobs_mtx);

	workqueue->workers = NULL;
	workqueue->waiting_jobs = NULL;

	pthread_cond_broadcast(&workqueue->jobs_cond);

	pthread_mutex_unlock(&workqueue->jobs_mtx);
	
}

void ntyThreadPoolQueue(nThreadPool *workqueue, nJob *job) {

	pthread_mutex_lock(&workqueue->jobs_mtx);

	LL_ADD(job, workqueue->waiting_jobs);
	
	pthread_cond_signal(&workqueue->jobs_cond);
	pthread_mutex_unlock(&workqueue->jobs_mtx);
	
}




/************************** debug thread pool **************************/
//sdk  --> software develop kit
// 提供SDK给其他开发者使用

#if 1

#define KING_MAX_THREAD			80
#define KING_COUNTER_SIZE		1000

void king_counter(nJob *job) {

	int index = *(int*)job->user_data;

	printf("index : %d, selfid : %lu\n", index, pthread_self());
	
	free(job->user_data);
	free(job);
}

/*
代码的缺点和需要改进的地方：

	没有实现线程的正确回收。在ntyWorkerThread函数中，free(worker)应在pthread_exit(NULL)之前调用。
	在ntyThreadPoolShutdown函数中，应该等待所有工作线程结束（使用pthread_join）。
	不应该直接设置workqueue->workers = NULL;在ntyThreadPoolShutdown中，否则可能会在工作线程中导致访问非法内存。
	ntyThreadPoolCreate函数中使用的pthread_cond_t和pthread_mutex_t的初始化方式不是最优的，应该直接调用pthread_cond_init和pthread_mutex_init。
	main函数中的工作函数king_counter不会正确地结束线程池，也不会清理已经分配的线程池资源。
	这段代码提供了一个基本的线程池框架，但在用于生产环境之前需要进行一些关键的改进和完善。

*/

int main(int argc, char *argv[]) {

	nThreadPool pool;

	ntyThreadPoolCreate(&pool, KING_MAX_THREAD);
	
	int i = 0;
	for (i = 0;i < KING_COUNTER_SIZE;i ++) {
		nJob *job = (nJob*)malloc(sizeof(nJob));
		if (job == NULL) {
			perror("malloc");
			exit(1);
		}
		
		job->job_function = king_counter;
		job->user_data = malloc(sizeof(int));
		*(int*)job->user_data = i;

		ntyThreadPoolQueue(&pool, job);
		
	}

	getchar();
	printf("\n");

	
}

#endif
