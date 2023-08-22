//  Task worker
//  Connects PULL socket to tcp://localhost:5557
//  Collects workloads from ventilator via that socket
//  Connects PUSH socket to tcp://localhost:5558
//  Sends results to sink via that socket

#include "zhelpers.h"



int main (void) 
{
    //  Socket to receive messages on
    void *context = zmq_ctx_new ();
    void *receiver = zmq_socket (context, ZMQ_PULL);
    zmq_connect (receiver, "tcp://localhost:5557");

    //  Process tasks forever
    while (1) {
        char *string = s_recv (receiver);
        printf ("%s.\n", string);       //  Show progress
        sleep (1);        //  Do the work

        free (string);
    }
    zmq_close (receiver);
    zmq_ctx_destroy (context);
    return 0;
}