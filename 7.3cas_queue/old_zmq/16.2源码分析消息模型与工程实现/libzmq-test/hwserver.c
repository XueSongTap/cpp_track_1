//
//  Hello World 服务端
//  绑定一个REP套接字至tcp://*:5555
//  从客户端接收Hello，并应答World
//

#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
//gcc -o hwserver hwserver.c -lzmq
int main (void)
{
    //  Socket to talk to clients
    void *context = zmq_ctx_new ();
    //  与客户端通信的套接字
    void *responder = zmq_socket (context, ZMQ_REP);
    int rc = zmq_bind (responder, "tcp://*:5555");  // 服务器要做绑定
    assert (rc == 0);
    char string [10];
    int count = 0;
    while (1) {
        //  等待客户端请求
        char buffer [20];
        int size = zmq_recv (responder, buffer, 20, 0);
        buffer[size] = '\0';
        int rcvmore;
        size_t sz = sizeof (rcvmore);
        zmq_getsockopt (responder, ZMQ_RCVMORE, &rcvmore, &sz);         // 多帧传输的时候需要判断是否还有多帧，本质是需要在循环里面判断的
        int size2 = 0;
        if(rcvmore) {
           size2 = zmq_recv (responder, &buffer[size], 20-size, 0);     // 客户端只发了2帧
        }
        buffer[size + size2] = '\0';
        zmq_getsockopt (responder, ZMQ_RCVMORE, &rcvmore, &sz);
        printf ("收到 %s， rcvmore:%d\n", buffer, rcvmore);
        sleep (1);          //  Do some 'work'
        //  返回应答
        sprintf (string, "World %d ", count++);
        zmq_send (responder,  string, strlen(string), 0);
    }
    return 0;
}