//============================================================================
// Name        : Message.cpp
// Author      : Roger Dickerson
// Version     :
// Copyright   : MIT License Copyright (c) 2017 dickerg-git
// Description : Message Que and iTimers in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace std;

#include <mqueue.h>
#include <signal.h>
#include <sys/time.h>

/* Function prototypes used in this project. */
#include "Thread.h"
extern mqd_t   mq_timer1;

extern unsigned int message_type;


/* Function to catch a SIGALRM event, then send a message to a task queue. */
void timer1_tick( int signum )
{
    struct timeval tod;
    int mq_stat;

    gettimeofday( &tod, NULL );
    // printf("%d.%06d: Timer1 tick\n", tod.tv_sec, tod.tv_usec);

    /* Send a message to each task message queue. */
    mq_stat = mq_send( mq_timer1, (const char*)&message_type, sizeof(message_type), 1);
}


int Message (unsigned int count_in)
{
    struct sigaction sig;
    struct itimerval timer1;
    // unsigned int count = 1;
    long long counter = 0;
    unsigned int i       = 0;
    unsigned int j       = 0;

    memset( &sig, 0, sizeof(sig) );
    sig.sa_handler = & timer1_tick;
    sigaction(SIGALRM, &sig, NULL);

    /* Set he initial timer value. */
    timer1.it_value.tv_sec = 0;
    timer1.it_value.tv_usec = 500000;
    /* Set the timer reload value. */
    timer1.it_interval.tv_sec = 1;
    timer1.it_interval.tv_usec = 0;

    /* Start the interval timer running... */
    setitimer(ITIMER_REAL, &timer1, NULL);
    /*
    * Crazy, the VIRTUAL I-timer speeds up as more threads/cores are added!!!
    * Three cores running, speeds up the I-timer 3 times...
    * Two cores running, speeds up the I-timer 2 times...
    * So the VIRTUAL timer adds all the time spent in all the threads???
    */


    /* Do some background work before returning */

    for (i = 0; i< 1000000; i++) { // Outer Loop 1M times
        for (j = 0; j< (10000*count_in); j++) { // Inner Loop 10K times
            counter++;
        }
    }

    /* Returning will cancel the timer1??? NO it keeps running!!! */
    return 0;
}
