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
/*
这段C代码实现了一个并发的echo服务器,使用了各种I/O多路复用技术比如select、poll和epoll。

主要步骤是:

1. 创建一个监听socket,并进行bind和listen。

2. 将监听socket添加到监控集合中 - select使用rfds,poll使用fds数组,epoll使用兴趣列表。

3. 进入循环,使用select()/poll()/epoll_wait()等待socket上的I/O事件。

4. 当监听socket就绪,表示有新的连接到来。接受连接并添加到监控集合。  

5. 当客户端socket就绪,表示有数据到达。接收数据,打印并原封不动写回去(echo)。

6. 将断开的socket从监控集合中删除。

一些要点:

- select()操作fd_sets,poll()操作pollfds数组,epoll操作兴趣列表。

- epoll在大量连接的时候效率最高。

- 代码展示了不同的方法 - 串行处理、每个客户端一个线程、select、poll、epoll。

总结一下,它展示了在C中使用I/O多路复用来构建可扩展的并发服务器的各种方法。
*/
#define MAXLNE  4096

#define POLL_SIZE	1024

//8m * 4G = 128 , 512
//C10k
void *client_routine(void *arg) { //

	int connfd = *(int *)arg;

	char buff[MAXLNE];

	while (1) {

		int n = recv(connfd, buff, MAXLNE, 0);
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);

	    	send(connfd, buff, n, 0);
        } else if (n == 0) {
            close(connfd);
			break;
        }

	}

	return NULL;
}


int main(int argc, char **argv) 
{
	/*
	这段代码是初始化服务器端监听socket的:

	1. socket() 创建TCP socket

	2. memset() 清空servaddr结构体

	3. 设置servaddr为IPv4协议,IP为INADDR_ANY(任意IP),端口为9999

	4. bind() 把socket和servaddr绑定

	5. listen() 设置监听队列长度为10

	这样就完成了服务器端监听socket的创建和初始化,监听9999端口等待客户端连接。

	后面可以通过accept()这个监听socket接收客户端连接,然后 recv()/send() 来收发数据实现网络通讯。

	整个流程包括:

	1. 创建监听socket

	2. 初始化socket地址结构体

	3. 绑定socket和地址

	4. 设置监听

	5. accept接收连接

	6. 收发数据

	这是基于TCP Socket的网络编程很基础的流程。
	*/
    int listenfd, connfd, n;
    struct sockaddr_in servaddr;
    char buff[MAXLNE];
 
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(9999);
 
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }
 
    if (listen(listenfd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

	/*
	这段代码被注释掉的是串行处理客户端连接的方式:

	1. accept() 接收一个客户端连接

	2. 在while循环中处理这个客户端
	- recv()接收数据
	- 打印接收到的数据
	- send()回复数据

	3. 当recv返回0时,表示客户端关闭,关闭connfd并退出循环

	这种方式的问题是:

	- 一次只能处理一个客户端连接
	- 不能同时处理多个客户端的请求

	所以不利于构建高并发服务器。

	而被注释掉的代码采用的是迭代处理方式:

	- accept接受连接
	- 处理连接
	- 关闭连接
	- 再accept下一个连接

	这种方式可以一个一个处理客户端连接,但仍然是串行的,不能并发。

	后面代码采用的select、poll、epoll等方式可以同时处理多个连接,这才能构建高并发服务器。

	总结一下:

	注释掉的代码采用迭代串行处理方式,仅处理一个客户端。
	后面代码使用I/O多路复用实现并发,可以同时处理多个客户端请求。
	*/
 #if 0
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
        return 0;
    }

    printf("========waiting for client's request========\n");
    while (1) {

        n = recv(connfd, buff, MAXLNE, 0);
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);

	    	send(connfd, buff, n, 0);
        } else if (n == 0) {
            close(connfd);
        }
        
        //close(connfd);
    }

#elif 0

	这段代码实现了服务器端迭代地处理客户端连接请求的流程:

	1. 调用accept函数接收一个客户端的连接请求,返回一个连接套接字connfd。

	2. 在while(1)循环中反复处理这个连接:

	- 调用recv函数从connfd套接字接收客户端发送过来的数据

	- 打印接收到的数据

	- 调用send函数将收到的数据原封不动发回给客户端(实现echo回显)

	- 如果recv返回0,表示客户端关闭了连接,关闭服务器端的connfd,退出循环

	3. 处理完一个客户端连接后,会跳出while循环,回到accept处再次等待新的客户端连接。

	所以这实现了迭代处理客户端连接的方式:接收连接,处理连接,关闭连接,循环下一个连接。

	关键点:

	- accept接收连接
	- recv/send与客户端通信 
	- 关闭连接并等待下一个连接

	这种方式可以一个一个处理客户端连接,但由于是阻塞的迭代处理,不能并发处理多个客户端,不适合高并发场景。

后面代码使用I/O多路复用(select/poll/epoll)实现同时处理多个连接的并发处理方式。
    printf("========waiting for client's request========\n");
    while (1) {

		struct sockaddr_in client;
	    socklen_t len = sizeof(client);
	    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
	        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
	        return 0;
	    }

        n = recv(connfd, buff, MAXLNE, 0);
        if (n > 0) {
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);

	    	send(connfd, buff, n, 0);
        } else if (n == 0) {
            close(connfd);
        }
        
        //close(connfd);
    }

#elif 0
	这段代码实现了为每个客户端连接创建一个新的线程来处理的方式:

	1. 在while循环中调用accept接收新客户端连接,返回连接套接字connfd。

	2. 为每个连接创建一个新的线程,线程函数是client_routine。

	3. 将connfd作为参数传递给线程函数client_routine。

	4. 在client_routine里面可以收发数据,处理这个连接。

	5. 主线程继续在while循环处理新的连接。

	这样通过为每个连接创建一个线程,可以实现并发地处理多个客户端连接,提高服务器效率。

	这种多线程方式的优点是 programming model简单,可以并发处理连接。

	缺点是线程资源 LIMITED,线程间切换开销大,不够高效。

	后面代码使用select/poll/epoll + 非阻塞IO和Reactor模式可以达到更高的并发效率。

	总结:

	这段代码通过为每个连接创建线程来并发处理多个连接请求,比迭代处理效率高,但线程资源有限制,还有优化空间。
	while (1) {

		struct sockaddr_in client;
	    socklen_t len = sizeof(client);
	    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
	        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
	        return 0;
	    }

		pthread_t threadid;
		pthread_create(&threadid, NULL, client_routine, (void*)&connfd);

    }

#elif 0
	这段代码使用了select()来实现I/O多路复用,可以并发处理多个客户端连接。

	主要逻辑是:

	1. 使用FD_SET将监听socket listenfd添加到读事件集合rfds中。

	2. 调用select监听rfds和wfds两个集合上的事件。

	3. 如果listenfd准备好了,表示有新连接,调用accept接受新连接,并添加到rfds集合监听。

	4. 遍历所有的客户端socket,如果可读,则 recv()收取数据,并添加到 wfds集合标记为可写,等待下一次loop时send()数据。

	5. 如果可写,则直接调用send()发回数据。

	6. 如果recv返回0,表示客户端关闭,关闭socket并从rfds 删除。

	这样通过select一个进程可以同时处理多个连接的读写事件,实现并发。

	优点是编程比多线程简单,没有多线程的切换消耗。

	缺点是连接数多时select效率下降。

	这个代码展示了使用select实现一个简单的并发服务器的整体流程。后面会使用poll和epoll实现更高效的多路复用。
	// 
	fd_set rfds, rset, wfds, wset;

	FD_ZERO(&rfds);
	FD_SET(listenfd, &rfds);

	FD_ZERO(&wfds);

	int max_fd = listenfd;

	while (1) {

		rset = rfds;
		wset = wfds;

		int nready = select(max_fd+1, &rset, &wset, NULL, NULL);


		if (FD_ISSET(listenfd, &rset)) { //

			struct sockaddr_in client;
		    socklen_t len = sizeof(client);
		    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
		        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
		        return 0;
		    }

			FD_SET(connfd, &rfds);

			if (connfd > max_fd) max_fd = connfd;

			if (--nready == 0) continue;

		}

		int i = 0;
		for (i = listenfd+1;i <= max_fd;i ++) {

			if (FD_ISSET(i, &rset)) { // 

				n = recv(i, buff, MAXLNE, 0);
		        if (n > 0) {
		            buff[n] = '\0';
		            printf("recv msg from client: %s\n", buff);

					FD_SET(i, &wfds);

					//reactor
					//send(i, buff, n, 0);
		        } else if (n == 0) { //

					FD_CLR(i, &rfds);
					//printf("disconnect\n");
		            close(i);
					
		        }
				if (--nready == 0) break;
			} else if (FD_ISSET(i, &wset)) {

				send(i, buff, n, 0);
				FD_SET(i, &rfds);
			
			}

		}
		

	}

#elif 0
这段代码使用了poll来实现I/O多路复用,原理类似select,但是可以监听更多的连接。

主要逻辑是:

1. 初始化一个 pollfd 结构体数组fds,添加监听socket,并注册POLLIN事件。

2. 在while循环中调用poll监听fds上的事件。

3. 如果监听socket有POLLIN事件,表示有新连接,调用accept处理。

4. 遍历poll返回的有事件的socket,如果是POLLIN事件,则调用recv收数据,并发送回复。

5. 如果是 socket关闭事件,则关闭socket。

6. 继续循环处理更多事件。

相比select,poll没有描述符数量的限制,可以监听更多连接。

poll和select编程模型类似,也需要在监听事件集合和socket间转换。

同样,连接数越大,效率也会下降。

这个代码展示了使用poll来实现一个简单的并发服务器。epoll可以进一步优化。

总结:

这段代码使用poll实现I/O多路复用,比select可以处理更多连接,实现一个简单的多连接并发服务器。

	struct pollfd fds[POLL_SIZE] = {0};
	fds[0].fd = listenfd;
	fds[0].events = POLLIN;

	int max_fd = listenfd;
	int i = 0;
	for (i = 1;i < POLL_SIZE;i ++) {
		fds[i].fd = -1;
	}

	while (1) {

		int nready = poll(fds, max_fd+1, -1);

	
		if (fds[0].revents & POLLIN) {

			struct sockaddr_in client;
		    socklen_t len = sizeof(client);
		    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
		        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
		        return 0;
		    }

			printf("accept \n");
			fds[connfd].fd = connfd;
			fds[connfd].events = POLLIN;

			if (connfd > max_fd) max_fd = connfd;

			if (--nready == 0) continue;
		}

		//int i = 0;
		for (i = listenfd+1;i <= max_fd;i ++)  {

			if (fds[i].revents & POLLIN) {
				
				n = recv(i, buff, MAXLNE, 0);
		        if (n > 0) {
		            buff[n] = '\0';
		            printf("recv msg from client: %s\n", buff);

					send(i, buff, n, 0);
		        } else if (n == 0) { //

					fds[i].fd = -1;

		            close(i);
					
		        }
				if (--nready == 0) break;

			}

		}

	}

#else

	//poll/select --> 
	// epoll_create 
	// epoll_ctl(ADD, DEL, MOD)
	// epoll_wait

	int epfd = epoll_create(1); //int size

	struct epoll_event events[POLL_SIZE] = {0};
	struct epoll_event ev;

	ev.events = EPOLLIN;
	ev.data.fd = listenfd;

	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);

	while (1) {

		int nready = epoll_wait(epfd, events, POLL_SIZE, 5);
		if (nready == -1) {
			continue;
		}

		int i = 0;
		for (i = 0;i < nready;i ++) {

			int clientfd =  events[i].data.fd;
			if (clientfd == listenfd) {

				struct sockaddr_in client;
			    socklen_t len = sizeof(client);
			    if ((connfd = accept(listenfd, (struct sockaddr *)&client, &len)) == -1) {
			        printf("accept socket error: %s(errno: %d)\n", strerror(errno), errno);
			        return 0;
			    }

				printf("accept\n");
				ev.events = EPOLLIN;
				ev.data.fd = connfd;
				epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev);

			} else if (events[i].events & EPOLLIN) {

				n = recv(clientfd, buff, MAXLNE, 0);
		        if (n > 0) {
		            buff[n] = '\0';
		            printf("recv msg from client: %s\n", buff);

					send(clientfd, buff, n, 0);
		        } else if (n == 0) { //


					ev.events = EPOLLIN;
					ev.data.fd = clientfd;

					epoll_ctl(epfd, EPOLL_CTL_DEL, clientfd, &ev);

		            close(clientfd);
					
		        }

			}

		}

	}
	

#endif
 
    close(listenfd);
    return 0;
}

