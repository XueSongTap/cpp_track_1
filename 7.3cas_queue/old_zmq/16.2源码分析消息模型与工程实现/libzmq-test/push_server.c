//  Task ventilator
//  Binds PUSH socket to tcp://localhost:5557
//  Sends batch of tasks to workers via that socket

#include "zhelpers.h"

int main (void) 
{
    void *context = zmq_ctx_new ();

    //  Socket to send messages on
    void *sender = zmq_socket (context, ZMQ_PUSH);
    zmq_bind (sender, "tcp://*:5557");

    printf ("Sending tasks to workers...\n");

    //  Initialize random number generator
    srandom ((unsigned) time (NULL));

    //  Send 100 tasks
    int task_nbr;
    int total_msec = 0;     //  Total expected cost in msecs
    unsigned short workload = 0;
    // for (task_nbr = 0; task_nbr < 100000; task_nbr++) 
    while(1)
    {
        
        //  Random workload from 1 to 100msecs
        workload +=  1;
        char string [30];
        sprintf (string, "work:%d", workload);
        
        int ret = s_send (sender, string);    // 用户层没有关注pull
        printf("ret:%d, %s\n", ret, string);
        // sleep(1);
    }
    printf ("send finish\n");

    zmq_close (sender);
    zmq_ctx_destroy (context);
    return 0;
}
