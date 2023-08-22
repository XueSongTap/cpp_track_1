//  Simple request-reply broker

#include "zhelpers.h"

int main (void) 
{
    //  Prepare our context and sockets
    void *context = zmq_ctx_new ();
    void *frontend = zmq_socket (context, ZMQ_ROUTER);      // ZMQ_REP继承了ZMQ_ROUTER
    void *backend  = zmq_socket (context, ZMQ_DEALER);      // ZMQ_REQ继承了ZMQ_DEALER
    zmq_bind (frontend, "tcp://*:5559");
    zmq_bind (backend,  "tcp://*:5560");

    //  Initialize poll set
    zmq_pollitem_t items [] = {
        { frontend, 0, ZMQ_POLLIN, 0 },
        { backend,  0, ZMQ_POLLIN, 0 }
    };
    //  Switch messages between sockets
    while (1) {
        zmq_msg_t message;
        zmq_poll (items, 2, -1);        // poll消息
        printf("xxx items [0].revents:%x, %x\n", items [0].revents, ZMQ_POLLIN);
        if (items [0].revents & ZMQ_POLLIN) {
            while (1) {
                //  Process all parts of the message
                zmq_msg_init (&message);
                zmq_msg_recv (&message, frontend, 0);                   // 从前端接收数据
                int more = zmq_msg_more (&message);
                printf("REQ: 0x%u\n", zmq_msg_routing_id(&message));
                zmq_msg_send (&message, backend, more? ZMQ_SNDMORE: 0);     // 发送给后端
                zmq_msg_close (&message);
                if (!more)
                    break;      //  Last message part
            }
        }
        if (items [1].revents & ZMQ_POLLIN) {
            while (1) {
                //  Process all parts of the message
                zmq_msg_init (&message);
                zmq_msg_recv (&message, backend, 0);                        // 从后端接收数据
                int more = zmq_msg_more (&message);
                printf("REP: 0x%u\n", zmq_msg_routing_id(&message));
                zmq_msg_send (&message, frontend, more? ZMQ_SNDMORE: 0);    // 发送给前端
                zmq_msg_close (&message);
                if (!more)
                    break;      //  Last message part
            }
        }
    }
    //  We never get here, but clean up anyhow
    zmq_close (frontend);
    zmq_close (backend);
    zmq_ctx_destroy (context);
    return 0;
}
