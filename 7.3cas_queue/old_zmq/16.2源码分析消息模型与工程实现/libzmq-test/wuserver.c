//  Weather update server
//  Binds PUB socket to tcp://*:5556
//  Publishes random weather updates

#include "zhelpers.h"

int main (void)
{
    //  Prepare our context and publisher
    void *context = zmq_ctx_new ();
    void *publisher = zmq_socket (context, ZMQ_PUB);
    int rc = zmq_bind (publisher, "tcp://*:5556");
    assert (rc == 0);

    //  Initialize random number generator
    srandom ((unsigned) time (NULL));
    int zipcode, temperature, relhumidity;
    temperature = 0;
    while (1) {
        //  Get values that will fool the boss
        int zipcode, temperature, relhumidity;
        zipcode     = randof (100000);
        // zipcode = 10001;
        if(++temperature < 0)
            temperature = 0;

        relhumidity = randof (50) + 10;

        //  Send message to all subscribers
        char update [20];
        sprintf (update, "%05d %d %d", zipcode, temperature, relhumidity);
        printf ("pub %05d %d %d\n", zipcode, temperature, relhumidity);
        s_send (publisher, update);     // 发布是不管有没有人订阅
        // sleep(1);
    }
    zmq_close (publisher);
    zmq_ctx_destroy (context);
    return 0;
}
