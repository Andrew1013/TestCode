#ifndef TIMERLIB_H
#define TIMERLIB_H


#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#define sec(x)      (1000 * x)
#define msec(x)       (x)

typedef  void(*TimerExpiryFunc)(union sigval);

typedef struct _TX_TIMER_STRUCT {
	char       					sztimer_name[20];
    timer_t    					timerid;
    struct 						timespec stInitialtime;
    struct 						timespec stRescheduletime;
	pthread_t  					tpid;
}Tx_TimerHandle;


bool Tx_TimerDelete(Tx_TimerHandle *timer);
bool Tx_TimerActivate(Tx_TimerHandle *timer);
Tx_TimerHandle *Tx_TimerCreate(char *timeName, 												 
									int msInitialTime,
									int msRescheduleTime,												
									bool   bAutoActivate,
									TimerExpiryFunc   ExpiredFun,
									void *arg);





#endif









