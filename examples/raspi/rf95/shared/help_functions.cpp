// -*- mode: C++ -*-
/*
 * help_functions.cpp
 *
 *  Created on: Jun 11, 2020
 *      Author: Tilman Glötzner
 */
// help functions
#include <help_functions.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

void print_scheduler(void)
{
   int schedType;

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
   case SCHED_FIFO:
	   printf("Scheduling Policy is SCHED_FIFO\n");
	   break;
   case SCHED_OTHER:
	   printf("Scheduling Policy is SCHED_OTHER\n");
	   break;
   case SCHED_RR:
	   printf("Scheduling Policy is SCHED_RR\n");
	   break;
   default:
	   printf("Scheduling Policy is UNKNOWN\n");
   }
}

void print_scope(void)
{
	pthread_attr_t tattr;
	int scope;
	int ret;

	/* get scope of thread */
	ret = pthread_attr_getscope(&tattr, &scope);
	switch(scope)
	{
	case PTHREAD_SCOPE_SYSTEM:
		printf("Scheduling Scope is SYSTEM\n");
		break;
	case PTHREAD_SCOPE_PROCESS:
		printf("Scheduling Scope is PROCESS\n");
		break;
	default:
		printf("Scheduling Scope is UNKNOWN\n");
	}
}
