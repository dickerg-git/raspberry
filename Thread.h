/*
 * Thread.h
 *
 * Created on  : Dec 22, 2017
 * Version     :
 * Copyright   : MIT License Copyright (c) 2017 dickerg-git
 * Author      : Roger Dickerson
 */

#ifndef THREAD_H_
#define THREAD_H_

/* Function to start a counting loop. */
void *loop_function(void* in);

/* Function to start a VIRTUAL ITIMER loop. */
int Message (unsigned int count);

#define TIMER1_QUEUE "/timer1_mq"



#endif /* THREAD_H_ */
