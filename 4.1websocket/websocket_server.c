


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <openssl/sha.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#define BUFFER_LENGTH		4096
#define MAX_EPOLL_EVENTS	1024
#define SERVER_PORT			8888
#define PORT_COUNT			100


#define 	GUID	"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"



/*
这是一份实现websocket服务器的代码,主要功能有:

1. 初始化反应堆ntyreactor,包含epoll和事件块。

2. 创建监听socket,添加到反应堆监听。

3. 定义websocket的数据帧头部结构。

4. 在接收回调recv_cb中,根据状态机解析websocket请求。

5. 在状态机为handshake时,完成握手校验。

6. 在状态机为transmission时,解析并处理websocket数据帧。

7. 在发送回调send_cb中直接发送缓存的数据。

8. 主循环驱动反应堆,处理事件。

9. 主进程创建多个监听端口,实现多进程模型。

10. 动态扩展内存以支持大量连接。

主要特点是:

- 使用反应堆模式处理并发
- 多进程模型提高并发性能
- 状态机解析websocket请求包
- 动态内存管理

这实现了一个可以处理websocket协议的多进程网络服务器。
通过反应堆+状态机解析websocket数据帧,并可以扩展到多进程,以支持海量websocket连接。
*/


/*
和http 的不同

这个websocket服务器代码与http服务器代码的主要不同之处在于:

1. 协议解析上:

 - http只需要解析请求头,获取请求方法、资源等。
 - websocket需要触发和完成复杂的握手流程,并解析自定义的数据帧格式。

2. 连接处理上:

 - http采用短连接,一个请求开启一个连接,响应结束后关闭。
 - websocket建立长连接,完成握手后连接持续存在。

3. 数据处理上:

 - http只处理一次请求和响应的数据。
 - websocket需要持续解析和处理无限的数据帧。 

4. 状态管理上:

 - http只有简单的请求/响应两种状态。
 - websocket有复杂的多种状态,需要状态机管理。

5. 数据格式上:

 - http采用文本格式。
 - websocket定义了二进制和文本帧,并有复杂的帧头。

6. 性能需求上:

 - websocket的长连接和数据量更大,对服务器性能要求更高。

总之,websocket相比http协议更复杂,连接更持久,交互更频繁,对服务器性能和消息处理也有更高要求。需要处理复杂的握手流程和自定义数据帧。
*/


/*
从代码实现层面来看,websocket服务器与http服务器在以下方面有重要区别:

1. 初始化阶段

- http只需要创建监听socket即可。

- websocket需要初始化各种状态,如状态机的值等。

2. 解析阶段

- http只需要简单的解析请求行、请求头等文本格式信息。

- websocket需要解析复杂的握手数据,实现校验,还需要解析自定义的二进制数据帧。

3. 连接处理

- http创建短连接,最简单直接返回写socket。

- websocket需要检测连接关闭并移除监听事件,还要实现心跳等定时检查。

4. 数据处理

- http一次性处理请求和响应的数据。

- websocket需要缓存并持续处理无限的数据帧。

5. 状态管理

- http只有请求和响应两个主要状态。

- websocket需要管理多个状态:握手、数据发送、关闭等。

6. 内存管理

- http可以直接返回写入数据。

- websocket需要缓存数据,不能直接返回写入。

7. 性能优化

- websocket需要注意超时控制,内存控制,以支持海量长连接。

所以从实现层面,websocket都比http复杂得多,需要处理更多的场景和状态,对代码的结构和性能要求都更高。这也是更大的工程实现难点。
*/

// 状态机取值，三种状态
enum {
	WS_HANDSHARK = 0,
	WS_TRANMISSION = 1,
	WS_END = 2,
};


typedef struct _ws_ophdr {
	
	unsigned char opcode:4,
				  rsv3:1,
				  rsv2:1,
				  rsv1:1,
				  fin:1;
	unsigned char pl_len:7,
				  mask:1;
} ws_ophdr;

typedef struct _ws_head_126 {

	unsigned short payload_length;
	char mask_key[4];

} ws_head_126;

typedef struct _ws_head_127 {

	long long payload_length;
	char mask_key[4];

} ws_head_127;



typedef int NCALLBACK(int ,int, void*);

struct ntyevent {
	int fd;
	int events;
	void *arg;
	int (*callback)(int fd, int events, void *arg);
	
	int status;
	char buffer[BUFFER_LENGTH];
	int length;
	long last_active;

	int status_machine; //定义了一个状态变量status_machine,用于表示websocket连接的当前状态:
};

struct eventblock {

	struct eventblock *next;
	struct ntyevent *events;
	
};

struct ntyreactor {
	int epfd;
	int blkcnt;
	struct eventblock *evblk; //fd --> 100w
};


int recv_cb(int fd, int events, void *arg);
int send_cb(int fd, int events, void *arg);
struct ntyevent *ntyreactor_idx(struct ntyreactor *reactor, int sockfd);


void nty_event_set(struct ntyevent *ev, int fd, NCALLBACK callback, void *arg) {

	ev->fd = fd;
	ev->callback = callback;
	ev->events = 0;
	ev->arg = arg;
	ev->last_active = time(NULL);
	
	return ;
	
}


int nty_event_add(int epfd, int events, struct ntyevent *ev) {

	struct epoll_event ep_ev = {0, {0}};
	ep_ev.data.ptr = ev;
	ep_ev.events = ev->events = events;

	int op;
	if (ev->status == 1) {
		op = EPOLL_CTL_MOD;
	} else {
		op = EPOLL_CTL_ADD;
		ev->status = 1;
	}

	if (epoll_ctl(epfd, op, ev->fd, &ep_ev) < 0) {
		printf("event add failed [fd=%d], events[%d]\n", ev->fd, events);
		return -1;
	}

	return 0;
}

int nty_event_del(int epfd, struct ntyevent *ev) {

	struct epoll_event ep_ev = {0, {0}};

	if (ev->status != 1) {
		return -1;
	}

	ep_ev.data.ptr = ev;
	ev->status = 0;
	epoll_ctl(epfd, EPOLL_CTL_DEL, ev->fd, &ep_ev);

	return 0;
}


int base64_encode(char *in_str, int in_len, char *out_str) {    
	BIO *b64, *bio;    
	BUF_MEM *bptr = NULL;    
	size_t size = 0;    

	if (in_str == NULL || out_str == NULL)        
		return -1;    

	b64 = BIO_new(BIO_f_base64());    
	bio = BIO_new(BIO_s_mem());    
	bio = BIO_push(b64, bio);
	
	BIO_write(bio, in_str, in_len);    
	BIO_flush(bio);    

	BIO_get_mem_ptr(bio, &bptr);    
	memcpy(out_str, bptr->data, bptr->length);    
	out_str[bptr->length-1] = '\0';    
	size = bptr->length;    

	BIO_free_all(bio);    
	return size;
}


int readline(char *allbuf, int idx, char *linebuf) {

	int len = strlen(allbuf);

	for(;idx < len;idx ++) {
		if (allbuf[idx] == '\r' && allbuf[idx+1] == '\n') {
			return idx+2;
		} else {
			*(linebuf++) = allbuf[idx];
		}
	}

	return -1;
}

/*

ev->buffer :
ev->length

GET / HTTP/1.1
Host: 192.168.232.128:8888
Connection: Upgrade
Pragma: no-cache
Cache-Control: no-cache
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/95.0.4638.69 Safari/537.36
Upgrade: websocket
Origin: null
Sec-WebSocket-Version: 13
Accept-Encoding: gzip, deflate
Accept-Language: zh-TW,zh;q=0.9,en-US;q=0.8,en;q=0.7
Sec-WebSocket-Key: QWz1vB/77j8J8JcT/qtiLQ==
Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits

str = "QWz1vB/77j8J8JcT/qtiLQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

sha = SHA1(str);

value = base64_encode(sha);


*/
// 19 : length of "Sec-WebSocket-Key: "
#define WEBSOCK_KEY_LENGTH	19
/*
WebSocket需要先以HTTP请求的形式发起连接,然后协议升级到WebSocket协议。这个过程就是WebSocket的握手阶段。

在这个代码示例中,握手过程是在recv_cb回调函数中的handshark函数里实现的,主要流程是:

1. 客户端发起HTTP请求,包含Upgrade等字段表明要升级协议。

2. 服务器端在recv_cb收到请求数据后,调用handshark函数。

3. handshark函数解析请求数据,提取Sec-WebSocket-Key字段。

4. 根据Sec-WebSocket-Key生成响应需要的Sec-WebSocket-Accept through计算。 

5. 构造握手响应数据,包含101状态码等WebSocket必要字段。

6. 将握手响应数据发送回客户端。

7. 接收回调设置状态机为WS_TRANMISSION,表示握手完成,进入数据传输状态。

所以整个从HTTP到WebSocket的协议升级握手过程就是在handshark函数里实现的。

这确保了每一个WebSocket连接都需要先完成HTTP握手,服务器验证了请求的合法性之后才会升级到WebSocket连接,然后再进行数据传输。

这样通过HTTP请求支持WebSocket的协议升级是它的一个重要设计。
*/

int handshark(struct ntyevent *ev) {

	//ev->buffer , ev->length

	char linebuf[1024] = {0};
	int idx = 0;
	char sec_data[128] = {0};
	char sec_accept[32] = {0};

	do {

		memset(linebuf, 0, 1024);
		idx = readline(ev->buffer, idx, linebuf);

		if (strstr(linebuf, "Sec-WebSocket-Key")) {

			//linebuf: Sec-WebSocket-Key: QWz1vB/77j8J8JcT/qtiLQ==
			strcat(linebuf, GUID);

			//linebuf: 
			//Sec-WebSocket-Key: QWz1vB/77j8J8JcT/qtiLQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11

			
			SHA1(linebuf + WEBSOCK_KEY_LENGTH, strlen(linebuf + WEBSOCK_KEY_LENGTH), sec_data); // openssl

			base64_encode(sec_data, strlen(sec_data), sec_accept);

			memset(ev->buffer, 0, BUFFER_LENGTH); 

			ev->length = sprintf(ev->buffer, "HTTP/1.1 101 Switching Protocols\r\n"
					"Upgrade: websocket\r\n"
					"Connection: Upgrade\r\n"
					"Sec-WebSocket-Accept: %s\r\n\r\n", sec_accept);

			printf("ws response : %s\n", ev->buffer);

			break;
			
		}

	} while((ev->buffer[idx] != '\r' || ev->buffer[idx+1] != '\n') && idx != -1 );

	return 0;
}

void umask(char *payload, int length, char *mask_key) {

	int i = 0;
	for (i = 0;i < length;i ++) {
		payload[i] ^= mask_key[i%4];
	}

}


int transmission(struct ntyevent *ev) {

	//ev->buffer; ev->length

	ws_ophdr *hdr = (ws_ophdr*)ev->buffer;

	printf("length: %d\n", hdr->pl_len);

	if (hdr->pl_len < 126) { //

		
		unsigned char *payload = ev->buffer + sizeof(ws_ophdr) + 4; // 6  payload length < 126
		if (hdr->mask) { // mask set 1

			umask(payload, hdr->pl_len, ev->buffer+2);
			
		}
		printf("payload : %s\n", payload);
		
	
	} else if (hdr->pl_len == 126) {

		ws_head_126 *hdr126 = ev->buffer + sizeof(ws_ophdr);

	} else {

		ws_head_127 *hdr127 = ev->buffer + sizeof(ws_ophdr);

	}

}


int websocket_request(struct ntyevent *ev) {

	if (ev->status_machine == WS_HANDSHARK) {
		// 调用握手处理函数
		ev->status_machine = WS_TRANMISSION; // 设置为传输状态
		handshark(ev);

	} else if (ev->status_machine == WS_TRANMISSION) {
		// 调用数据帧处理函数
		transmission(ev);

	} else {
	
	}

	printf("websocket_request --> %d\n", ev->status_machine);
	
}

int recv_cb(int fd, int events, void *arg) {

	struct ntyreactor *reactor = (struct ntyreactor*)arg;
	struct ntyevent *ev = ntyreactor_idx(reactor, fd);

	int len = recv(fd, ev->buffer, BUFFER_LENGTH , 0); // 
	
	if (len > 0) {
		
		ev->length = len;
		ev->buffer[len] = '\0';

		printf("C[%d]: machine: %d\n", fd, ev->status_machine);

		websocket_request(ev);

		nty_event_del(reactor->epfd, ev);
		nty_event_set(ev, fd, send_cb, reactor);
		nty_event_add(reactor->epfd, EPOLLOUT, ev);
		
		
	} else if (len == 0) {

		nty_event_del(reactor->epfd, ev);
		close(ev->fd);
		
		//printf("[fd=%d] pos[%ld], closed\n", fd, ev-reactor->events);
		 
	} else {

		nty_event_del(reactor->epfd, ev);
		close(ev->fd);
		printf("recv[fd=%d] error[%d]:%s\n", fd, errno, strerror(errno));
		
	}

	return len;
}


int send_cb(int fd, int events, void *arg) {

	struct ntyreactor *reactor = (struct ntyreactor*)arg;
	struct ntyevent *ev = ntyreactor_idx(reactor, fd);

	int len = send(fd, ev->buffer, ev->length, 0);
	if (len > 0) {
		printf("send[fd=%d], [%d]%s\n", fd, len, ev->buffer);

		nty_event_del(reactor->epfd, ev);
		nty_event_set(ev, fd, recv_cb, reactor);
		nty_event_add(reactor->epfd, EPOLLIN, ev);
		
	} else {

		close(ev->fd);

		nty_event_del(reactor->epfd, ev);
		printf("send[fd=%d] error %s\n", fd, strerror(errno));

	}

	return len;
}

int accept_cb(int fd, int events, void *arg) {

	struct ntyreactor *reactor = (struct ntyreactor*)arg;
	if (reactor == NULL) return -1;

	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	int clientfd;

	if ((clientfd = accept(fd, (struct sockaddr*)&client_addr, &len)) == -1) {
		if (errno != EAGAIN && errno != EINTR) {
			
		}
		printf("accept: %s\n", strerror(errno));
		return -1;
	}

	

	int flag = 0;
	if ((flag = fcntl(clientfd, F_SETFL, O_NONBLOCK)) < 0) {
		printf("%s: fcntl nonblocking failed, %d\n", __func__, MAX_EPOLL_EVENTS);
		return -1;
	}

	struct ntyevent *event = ntyreactor_idx(reactor, clientfd);//在accept回调中,会为新连接初始化状态机的值: 设置为握手状态,等待握手数据的到来。

	event->status_machine = WS_HANDSHARK;
	nty_event_set(event, clientfd, recv_cb, reactor);
	nty_event_add(reactor->epfd, EPOLLIN, event);
	
	
	printf("new connect [%s:%d], pos[%d]\n", 
		inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), clientfd);

	return 0;

}

int init_sock(short port) {

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	fcntl(fd, F_SETFL, O_NONBLOCK);

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	bind(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));

	if (listen(fd, 20) < 0) {
		printf("listen failed : %s\n", strerror(errno));
	}

	return fd;
}


int ntyreactor_alloc(struct ntyreactor *reactor) {

	if (reactor == NULL) return -1;
	if (reactor->evblk == NULL) return -1;

	struct eventblock *blk = reactor->evblk;
	while (blk->next != NULL) {
		blk = blk->next;
	}

	struct ntyevent *evs = (struct ntyevent*)malloc((MAX_EPOLL_EVENTS) * sizeof(struct ntyevent));
	if (evs == NULL) {
		printf("ntyreactor_alloc ntyevents failed\n");
		return -2;
	}
	memset(evs, 0, (MAX_EPOLL_EVENTS) * sizeof(struct ntyevent));

	struct eventblock *block = (struct eventblock *)malloc(sizeof(struct eventblock));
	if (block == NULL) {
		printf("ntyreactor_alloc eventblock failed\n");
		return -2;
	}
	memset(block, 0, sizeof(struct eventblock));

	block->events = evs;
	block->next = NULL;

	blk->next = block;
	reactor->blkcnt ++; //

	return 0;
}

struct ntyevent *ntyreactor_idx(struct ntyreactor *reactor, int sockfd) {

	int blkidx = sockfd / MAX_EPOLL_EVENTS;

	while (blkidx >= reactor->blkcnt) {
		ntyreactor_alloc(reactor);
	}

	int i = 0;
	struct eventblock *blk = reactor->evblk;
	while(i ++ < blkidx && blk != NULL) {
		blk = blk->next;
	}

	return &blk->events[sockfd % MAX_EPOLL_EVENTS];
}


int ntyreactor_init(struct ntyreactor *reactor) {

	if (reactor == NULL) return -1;
	memset(reactor, 0, sizeof(struct ntyreactor));

	reactor->epfd = epoll_create(1);
	if (reactor->epfd <= 0) {
		printf("create epfd in %s err %s\n", __func__, strerror(errno));
		return -2;
	}

	struct ntyevent *evs = (struct ntyevent*)malloc((MAX_EPOLL_EVENTS) * sizeof(struct ntyevent));
	if (evs == NULL) {
		printf("ntyreactor_alloc ntyevents failed\n");
		return -2;
	}
	memset(evs, 0, (MAX_EPOLL_EVENTS) * sizeof(struct ntyevent));

	struct eventblock *block = (struct eventblock *)malloc(sizeof(struct eventblock));
	if (block == NULL) {
		printf("ntyreactor_alloc eventblock failed\n");
		return -2;
	}
	memset(block, 0, sizeof(struct eventblock));

	block->events = evs;
	block->next = NULL;

	reactor->evblk = block;
	reactor->blkcnt = 1;

	return 0;
}

int ntyreactor_destory(struct ntyreactor *reactor) {

	close(reactor->epfd);
	//free(reactor->events);

	struct eventblock *blk = reactor->evblk;
	struct eventblock *blk_next = NULL;

	while (blk != NULL) {

		blk_next = blk->next;

		free(blk->events);
		free(blk);

		blk = blk_next;

	}
	
	return 0;
}



int ntyreactor_addlistener(struct ntyreactor *reactor, int sockfd, NCALLBACK *acceptor) {

	if (reactor == NULL) return -1;
	if (reactor->evblk == NULL) return -1;

	//reactor->evblk->events[sockfd];
	struct ntyevent *event = ntyreactor_idx(reactor, sockfd);

	nty_event_set(event, sockfd, acceptor, reactor);
	nty_event_add(reactor->epfd, EPOLLIN, event);

	return 0;
}



int ntyreactor_run(struct ntyreactor *reactor) {
	if (reactor == NULL) return -1;
	if (reactor->epfd < 0) return -1;
	if (reactor->evblk == NULL) return -1;
	
	struct epoll_event events[MAX_EPOLL_EVENTS+1];
	
	int checkpos = 0, i;

	while (1) {
/*
		long now = time(NULL);
		for (i = 0;i < 100;i ++, checkpos ++) {
			if (checkpos == MAX_EPOLL_EVENTS) {
				checkpos = 0;
			}

			if (reactor->events[checkpos].status != 1) {
				continue;
			}

			long duration = now - reactor->events[checkpos].last_active;

			if (duration >= 60) {
				close(reactor->events[checkpos].fd);
				printf("[fd=%d] timeout\n", reactor->events[checkpos].fd);
				nty_event_del(reactor->epfd, &reactor->events[checkpos]);
			}
		}
*/

		int nready = epoll_wait(reactor->epfd, events, MAX_EPOLL_EVENTS, 1000);
		if (nready < 0) {
			printf("epoll_wait error, exit\n");
			continue;
		}

		for (i = 0;i < nready;i ++) {

			struct ntyevent *ev = (struct ntyevent*)events[i].data.ptr;

			if ((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)) {
				ev->callback(ev->fd, events[i].events, ev->arg);
			}
			if ((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)) {
				ev->callback(ev->fd, events[i].events, ev->arg);
			}
			
		}

	}
}

// 3, 6w, 1, 100 == 
// <remoteip, remoteport, localip, localport>
int main(int argc, char *argv[]) {

	unsigned short port = SERVER_PORT; // listen 8888
	if (argc == 2) {
		port = atoi(argv[1]);
	}
	struct ntyreactor *reactor = (struct ntyreactor*)malloc(sizeof(struct ntyreactor));
	ntyreactor_init(reactor);

	int i = 0;
	int sockfds[PORT_COUNT] = {0};
	for (i = 0;i < PORT_COUNT;i ++) {
		sockfds[i] = init_sock(port+i);
		ntyreactor_addlistener(reactor, sockfds[i], accept_cb);
	}

	
	ntyreactor_run(reactor);

	ntyreactor_destory(reactor);

	for (i = 0;i < PORT_COUNT;i ++) {
		close(sockfds[i]);
	}

	free(reactor);

	return 0;
}



