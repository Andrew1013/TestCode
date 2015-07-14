
#include "timerlib.h"

bool Tx_TimerDelete(Tx_TimerHandle *timer)
{    
	if(timer == NULL)
    	return false; 	
    timer_delete(timer->timerid);
    free(timer);
    return true;
}

bool  Tx_TimerActivate(Tx_TimerHandle *timer)
{    
    if(timer == NULL)
    	return false; 
    struct itimerspec interval;
    memcpy(&interval.it_interval, &timer->stRescheduletime, sizeof(struct timespec));
    memcpy(&interval.it_value, &timer->stInitialtime,  sizeof(struct timespec));
    timer_settime(timer->timerid, 0, &interval, NULL);    
    return true;
}


Tx_TimerHandle *Tx_TimerCreate(char *timeName, 
												struct timespec stInitialtime, 												
												struct timespec stRescheduletime,
												bool   bAutoActivate,
												TimerExpiryFunc   ExpiredFun,
												void *arg)
{   
    struct sigevent   event; 
    int res;
    Tx_TimerHandle *newTimer = malloc(sizeof(struct _TX_TIMER_STRUCT));  
    
    if(newTimer == NULL)
    	return NULL; 
    
    memset(&event, 0, sizeof(struct sigevent));  
    event.sigev_notify = SIGEV_THREAD;
    
    if(arg == NULL)  
    	event.sigev_value.sival_ptr = newTimer;  
    else	
    	event.sigev_value.sival_ptr = arg; 
    	
    event.sigev_notify_function = ExpiredFun;
    event.sigev_notify_attributes = NULL;
    newTimer->tpid = pthread_self();
    snprintf(newTimer->sztimer_name, sizeof(newTimer->sztimer_name), "%s",timeName );
    memcpy(&newTimer->stInitialtime , &stInitialtime, sizeof(struct timespec));
    memcpy(&newTimer->stRescheduletime , &stRescheduletime, sizeof(struct timespec));
    
    res = timer_create(CLOCK_MONOTONIC, &event, &(newTimer->timerid)); 
    if(res != 0)
    {    	
    	free(newTimer);
    	return NULL;
    }	
    
    if(bAutoActivate)
 			Tx_TimerActivate(newTimer);
 		
    return newTimer;
}













