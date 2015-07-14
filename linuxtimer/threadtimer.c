#include "timerlib.h"
#include <unistd.h>
#include <stdio.h>

typedef struct _sAAASTR_Tag
{
	int a;
	char str[1024*1024];
	
}sAAASTR;

static void timerFunction(union sigval sv) 
{ 
    if(sv.sival_ptr != NULL)
    {
        Tx_TimerHandle *pTimehandle = (Tx_TimerHandle *)(sv.sival_ptr);
        pthread_t	tpid = pTimehandle->tpid; 
        Tx_TimerDelete(pTimehandle);	
        pthread_cancel(tpid);
    }
}

static void threadFunction3(void *arg) 
{
	struct timespec Inittime;
	struct timespec Rescheduletime;	
    
	sAAASTR  *pAaaStr= malloc(sizeof(sAAASTR)) ;
	pAaaStr->a = 9999;
	strcpy(pAaaStr->str, "threadFunction3"); 	
	
	Inittime.tv_sec = 5;
	Inittime.tv_nsec = 0;
	Rescheduletime.tv_sec = 0;
	Rescheduletime.tv_nsec = 0;		
	Tx_TimerHandle *time3 = Tx_TimerCreate("Time3", Inittime, Rescheduletime, true, timerFunction,  NULL);		
	Tx_TimerDelete(time3);
	pthread_exit( (void *)pAaaStr );
}


static void threadFunction4(void *arg) 
{
	struct timespec Inittime;
	struct timespec Rescheduletime;	
    int i = 0;
    int iSec = 0;    
    int *retval = malloc(sizeof(int));
    *retval = 1;
    
    if(arg != NULL) 
        iSec = *((int*)arg);       

	Inittime.tv_sec = 5;
	Inittime.tv_nsec = 0;
	Rescheduletime.tv_sec = 0;
	Rescheduletime.tv_nsec = 0;		
	Tx_TimerHandle *time3 = Tx_TimerCreate("Time4", Inittime, Rescheduletime, true, timerFunction,  NULL);		
	
	for(i=0 ; i< iSec; i++)
	{		
		sleep(1);
	}
	
	Tx_TimerDelete(time3);     
	pthread_exit((void *) retval );
	
}


void threadFunction1(union sigval sv) 
{
    if(sv.sival_ptr != NULL)
        printf("%s,%s\n",__FUNCTION__,(char*) sv.sival_ptr);
}

void threadFunction2(union sigval sv) 
{
    if(sv.sival_ptr != NULL)
        printf("%s,%s\n",__FUNCTION__,(char*) sv.sival_ptr);
}

int main()
{
	char cstr1[100] = "Timer1";
	char cstr2[100] = "Timer2";
	struct timespec Inittime;
	struct timespec Rescheduletime;
    Tx_TimerHandle *pTimeHandle;
    void *p_retval;
    pthread_t a_thread;
    int iRet = 0;
    int isec = 0;
   
    printf("------ Run Timer1 once ------ \n");
	Inittime.tv_sec = 1;
	Inittime.tv_nsec = 0;
	Rescheduletime.tv_sec = 0;
	Rescheduletime.tv_nsec = 0;	
	Tx_TimerCreate("Time1", Inittime, Rescheduletime, true, threadFunction1, cstr1);
    sleep(3);
    
    printf("------ Run Timer2 multi times ------ \n");
	Inittime.tv_sec = 3;
	Inittime.tv_nsec = 0;
	Rescheduletime.tv_sec = 3;
	Rescheduletime.tv_nsec = 0;
	pTimeHandle = Tx_TimerCreate("Time2", Inittime, Rescheduletime, true, threadFunction2, cstr2);
    sleep(15);
    Tx_TimerDelete(pTimeHandle);    
    sleep(5);

    printf("------ Create one thread and Timer3 thread to monitor (without timeout) ------ \n");
    pthread_create(&a_thread, NULL, (void *)&threadFunction3, (void*) &isec);
    iRet = pthread_join(a_thread, &p_retval);	
    if(iRet == 0 && p_retval != NULL)
    {
        sAAASTR *p_param =(sAAASTR*) p_retval;  
        printf("Thread output [%s:%d]\n", p_param->str, p_param->a);
        free(p_param);
    }    
   
    printf("------ Create one thread and Timer4 thread to monitor (with timeout) ------ \n");
    isec = 10;
    pthread_create(&a_thread, NULL, (void *)&threadFunction4, (void*) &isec);
    iRet = pthread_join(a_thread, &p_retval);    

    if((intptr_t)  0xffffffffffffffff == (intptr_t)  p_retval)
    {
        printf("Thread timeout.... \n");    
    }
    else
    {
        int *a = ((int*) p_retval);
        printf("Thread run successfully, retval:%d\n", *a);
        free(p_retval);
    }
    
    printf("Done.......\n");    
	return 0;		
  
}





