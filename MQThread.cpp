//============================================================================
// Name        : RGDThread.cpp
// Author      : Roger Dickerson
// Version     :
// Copyright   : MIT License Copyright (c) 2017 dickerg-git
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

#include <pthread.h> /* compile add -lpthread */
#include <stdlib.h>

/* Function prototypes used in this project. */
#include "Thread.h"
#include <mqueue.h>  /* compile add -lrt     */
#include <fcntl.h>

struct mq_attr mq_timer1_attr;
       mqd_t   mq_timer1;

unsigned int message_type = 0x2000;


/* Set up Global Mutex for multi-threading operations. */
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
long long Task_Counter = 0;


void *loop_function(void* in) {

    long long counter = 0;
    // int i       = 0;
    int j       = 0;
    int count_in = *(int*)in;
    unsigned int rcv_message_type = 0;

    //for (i = 0; i< 1000000; i++) { // Outer Loop 1M times
    if ( count_in > 0 ) {
        for (j = 0; j< (100*count_in); j++) { // Inner Loop 10K times
            counter++;

            /* Start a message queue receive call.                                  */
            /* As expected, the thread waits here, but the processor load is small. */
            mq_receive( mq_timer1, (char*)&rcv_message_type, sizeof(rcv_message_type), NULL );

            cout << "Message to task: " << count_in << endl;
        }
    }
    cout << "Ending loop_function..." << endl;

    return ( (void *) 0);

} // End of loop_function


int print_main(void);

int main() {
unsigned int count = 1;

/* Variables used for mutli-threading tasks. */
int rc1, arg1 = 2;
int rc2, arg2 = 3;
pthread_t thread1, thread2;

        while (count--) {
           print_main();
        }

        /* Set up a message queue to send timer1 ticks to threads. */
        mq_timer1_attr.mq_maxmsg = 10;
        mq_timer1_attr.mq_msgsize = sizeof(message_type);

        mq_timer1 = mq_open( TIMER1_QUEUE, (O_CREAT | O_RDWR ), 0666, &mq_timer1_attr );

		/* Create a worker thread to do busy work. */
        rc1 = pthread_create( &thread1, NULL, &loop_function, (void*)&arg1 );
        if (rc1) cout << "Thread1 create failed!!!" << endl;
        else cout << "Starting Thread1..." << endl;
        // pthread_join(thread1, NULL); //This will pend until thread1 exits!

        /* Create a worker thread to do busy work. */
        rc2 = pthread_create( &thread2, NULL, &loop_function, (void*)&arg2 );
        if (rc2) cout << "Thread2 create failed!!!" << endl;
        else cout << "Starting Thread2..." << endl;
        // pthread_join(thread2, NULL); //This will pend until thread2 exits!

        /* Start background work in main function. */
        count = 1;
        cout << "Starting background task..." << endl;

        /* From Message.cpp */
        /* This calls a function that starts a sigaction I-timer.
         * The loop_function is called there after starting the I-timer.
         *
         */
        Message(count);

        cout << "Ending background task..." << endl;

        /* Wait for the threads to complete. */
        pthread_join(thread1, NULL);
        pthread_join(thread2, NULL);

        cout << "Finished." << endl;
        return 0;
}




