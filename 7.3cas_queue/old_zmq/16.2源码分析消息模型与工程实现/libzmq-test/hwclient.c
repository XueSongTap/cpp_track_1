//
//  Hello World 客户端
//  连接REQ套接字至 tcp://localhost:5555
//  发送Hello给服务端，并接收World
//
//  Hello World client
#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
//编译：gcc -o hwclient hwclient.c -lzmq
int main (void)
{
    printf ("Connecting to hello world server...\n");
    void *context = zmq_ctx_new ();
    //  连接至服务端的套接字
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");

    int request_nbr;
    int ret = 0;
    for (request_nbr = 0; request_nbr != 10; request_nbr++) {
        char buffer [20];
        printf ("正在发送1 Hello %d...\n", request_nbr);
        // 将Hello darren消息分两个帧发送
        ret = zmq_send (requester, "Hello", 5, ZMQ_SNDMORE);            // 第一帧 flag设置为ZMQ_SNDMORE，告诉底层还有其他帧要发送
        printf ("zmq_send1 ret:%d\n", ret);
        ret = zmq_send (requester, " darren", strlen(" darren"), 0);    // 第二帧，如果是结束帧 flag设置为0
        printf ("zmq_send2 ret:%d\n", ret);
        ret = zmq_recv (requester, buffer, 20, 0); 
        buffer[ret] = '\0';
        printf ("ret:%d, 接收到 %s, %d\n",ret, buffer,request_nbr);
    }
    zmq_close (requester);
    zmq_ctx_destroy (context);
    return 0;
}