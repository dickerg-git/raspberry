//============================================================================
// Name        : RGD1.cpp
// Author      : Roger Dickerson
// Version     :
// Copyright   : MIT License Copyright (c) 2017 dickerg-git
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
using namespace std;

#include <pthread.h>
#include <stdlib.h>

/* Set up Global Mutex for multi-threading operations. */
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
long long Task_Counter = 0;


void *loop_function(void* in) {

	long long counter = 0;
	int i       = 0;
	int j       = 0;
    int count_in = *(int*)in;

	for (i = 0; i< 1000000; i++) { // Outer Loop 1M times

		for (j = 0; j< (10000*count_in); j++) { // Inner Loop 10K times
			counter++;
		}
    }
    cout << "Ending loop_function..." << endl;

	return ( (void *) 0);

} // End of loop_function



int main() {
unsigned int count = 1;

/* Variables used for mutli-threading tasks. */
int rc1, arg1 = 2;
int rc2, arg2 = 3;
pthread_t thread1, thread2;

	while (count--) {
	cout << "!!!Hello Suse Linux using Bear-test!!!" << endl;
	}

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
	loop_function( (void*)&count );
    cout << "Ending background task..." << endl;

    /* Wait for the threads to complete. */
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	cout << "Finished." << endl;
	return 0;
}
