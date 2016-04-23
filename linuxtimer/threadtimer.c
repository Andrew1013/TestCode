#include "timerlib.h"
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

volatile long long int count=0; 


typedef struct _sAAASTR_Tag
{
	int a;
	char str[1024*1024];
	
}sAAASTR;

static void timerFunction(union sigval sv) 
{ 
    int res = -1;
    int i = 0;
        
    if(sv.sival_ptr != NULL)
    {
        Tx_TimerHandle *pTimehandle = (Tx_TimerHandle *)(sv.sival_ptr);
        pthread_t	tpid = pTimehandle->tpid; 
        Tx_TimerDelete(pTimehandle);

        for(i = 0; i < 5; i++)
        {
            if((res = pthread_cancel(tpid)) == 0)
            {
                printf("Send pthread_cancel success\n");
            }
            sleep(1);
            if((res = pthread_cancel(tpid)) == 0)
            {
                printf("Send pthread_cancel success\n");
            }
            else
            {                
                if(res == ESRCH)
                {                    
                    printf("The pthread does not exist.\n");                    
                    return;
                }    
            }   
       }   
       printf("The pthread cancel failed...\n");    
       _exit(-1); 
    }
}

static void threadFunction3(void *arg) 
{
   
	sAAASTR  *pAaaStr= malloc(sizeof(sAAASTR)) ;
	pAaaStr->a = 9999;
	strcpy(pAaaStr->str, "threadFunction3"); 	
	
	
	Tx_TimerHandle *time3 = Tx_TimerCreate("Time3",  sec(5), 0, true, timerFunction,  NULL);		
	Tx_TimerDelete(time3);
	pthread_exit( (void *)pAaaStr );
}


static void threadFunction4(void *arg) 
{    
    int *retval = malloc(sizeof(int));
    *retval = 99;
    
 	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); 
 	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);     

	Tx_TimerHandle *time3 = Tx_TimerCreate("Time4", sec(5), 0, true, timerFunction,  NULL);		
	
 	while (1)
 	{ 
 	    count ++;
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
    Tx_TimerHandle *pTimeHandle;
    void *p_retval;
    pthread_t a_thread;
    int iRet = 0;
    int isec = 0;
    int i = 0;
   
    printf("------ Run Timer1 once ------ \n");
	pTimeHandle = Tx_TimerCreate("Time1", sec(1), 0, false, threadFunction1, cstr1);
    sleep(5);
	Tx_TimerActivate(pTimeHandle);
    sleep(3);
    Tx_TimerDelete(pTimeHandle);  
    
    printf("------ Run Timer2 multi times ------ \n");
	pTimeHandle = Tx_TimerCreate("Time2", sec(3), sec(3), true, threadFunction2, cstr2);
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
    pthread_create(&a_thread, NULL, (void *)&threadFunction4, (void*) &isec);
	for (i=0; i<5; i++) 
	{ 
		sleep(1); 
		printf("count=%lld\n", count); 
	}         
    iRet = pthread_join(a_thread, &p_retval);    
	for (i=0; i<5; i++) 
	{ 
		sleep(1); 
		printf("count=%lld\n", count); 
	} 


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





