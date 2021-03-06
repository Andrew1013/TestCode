#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "sem_rw_lib.h"


#define COMMON_LOCK_PATH    "./test10049"
#define RWLOCK_PATH         "./test28049"


void CommonTest1(void* argInfo)
{
    int sem_key = -1;
    bool ret = false;

    sem_key =Sem_LockInit(COMMON_LOCK_PATH);    
    //Sem_PrintInfo(sem_key);
    printf("[%s:%d] sem key:%d\n",__FUNCTION__, __LINE__,sem_key);    
    if(sem_key >= 0)
    {
        printf("[%s:%d] Try Lock Time:FOREVER\n",__FUNCTION__, __LINE__);
        if( Sem_TimedLock(sem_key, FOREVER))
        {
            printf("[%s:%d] Get Lock, sleep:20s\n",__FUNCTION__, __LINE__);            
            sleep(20);              
            ret = Sem_UnLock(sem_key);            
            printf("[%s:%d]  UnLock, ret:%d\n",__FUNCTION__, __LINE__, ret);
            
        }  
        else
        {
            printf("[%s:%d] Lock Failed\n",__FUNCTION__, __LINE__);
        }        
    }    
}

void CommonTest2(void* argInfo)
{
    int sem_key = -1;
    bool ret = false;
    sem_key =Sem_LockInit(COMMON_LOCK_PATH);   
    //Sem_PrintInfo(sem_key);
    printf("[%s:%d] sem key:%d\n",__FUNCTION__, __LINE__,sem_key);    
    if(sem_key >= 0)
    {   
        printf("[%s:%d] Try Lock Time:3s\n",__FUNCTION__, __LINE__);        
        if( Sem_TimedLock(sem_key, sec(3)))
        {            
            printf("[%s:%d] Get Lock, sleep:10s\n",__FUNCTION__, __LINE__);
            sleep(10);            
            ret = Sem_UnLock(sem_key);            
            printf("[%s:%d] UnLock, ret:%d\n",__FUNCTION__, __LINE__, ret);
        }
        else
        {
            printf("[%s:%d] Lock Failed\n",__FUNCTION__, __LINE__);
        }
    }  
}


bool Sem_CommonTest()
{   
    static pthread_t a_thread = (pthread_t) 0;
    pthread_attr_t sPthreadAttr;
    int i32Ret = -1;
    
    if (pthread_attr_init(&sPthreadAttr) == 0)
    {
        if (pthread_attr_setdetachstate(&sPthreadAttr, PTHREAD_CREATE_DETACHED) == 0)
        {
            if ((i32Ret = pthread_create(&a_thread, 
                                        &sPthreadAttr, 
                                        (void *(*)(void *)) CommonTest1, 
                                        (void *) NULL)) != 0)
            {                
                printf("%s() - Failed to create thread...", __FUNCTION__);                
                return false;
            }
        }    
    }

    if (pthread_attr_init(&sPthreadAttr) == 0)
    {
        if (pthread_attr_setdetachstate(&sPthreadAttr, PTHREAD_CREATE_DETACHED) == 0)
        {
            if ((i32Ret = pthread_create(&a_thread, 
                                        &sPthreadAttr, 
                                        (void *(*)(void *)) CommonTest2, 
                                        (void *) NULL)) != 0)
            {                
                printf("%s() - Failed to create thread...", __FUNCTION__);                
                return false;
            }
        }    
    }
  
    return true;
}    

void RlockTest1(void* argInfo)
{
    int sem_key = -1;
    bool ret = false;
    sem_key =Sem_RWLockInit(RWLOCK_PATH);   
    //Sem_PrintInfo(sem_key);
    sleep(1);
    
    printf("[%s:%d] sem key:%d\n",__FUNCTION__, __LINE__,sem_key);    
    if(sem_key >= 0)
    {   
        printf("[%s:%d] Try Lock Time:5s\n",__FUNCTION__, __LINE__);        
        if( Sem_TimedRdLock(sem_key, sec(5)))
        {            
            printf("[%s:%d] Get Lock, sleep:2s\n",__FUNCTION__, __LINE__);
            sleep(2);            
            ret = Sem_RdUnLock(sem_key);            
            printf("[%s:%d] UnLock, ret:%d\n",__FUNCTION__, __LINE__, ret);
        }
        else
        {
            printf("[%s:%d] Lock Failed\n",__FUNCTION__, __LINE__);
        }
    }  
}

void RlockTest2(void* argInfo)
{
    int sem_key = -1;
    bool ret = false;
    sem_key =Sem_RWLockInit(RWLOCK_PATH);  
    //Sem_PrintInfo(sem_key);
    sleep(1);
    
    printf("[%s:%d] sem key:%d\n",__FUNCTION__, __LINE__,sem_key);    
    if(sem_key >= 0)
    {   
        printf("[%s:%d] Try Lock Time:5s\n",__FUNCTION__, __LINE__);        
        if( Sem_TimedRdLock(sem_key, sec(5)))
        {            
            printf("[%s:%d] Get Lock, sleep:2s\n",__FUNCTION__, __LINE__);
            sleep(2);            
            ret = Sem_RdUnLock(sem_key);            
            printf("[%s:%d] UnLock, ret:%d\n",__FUNCTION__, __LINE__, ret);
        }
        else
        {
            printf("[%s:%d] Lock Failed\n",__FUNCTION__, __LINE__);
        }
    }   
}

void RlockTest3(void* argInfo)
{
    int sem_key = -1;
    bool ret = false;
    sem_key =Sem_RWLockInit(RWLOCK_PATH);    
    //Sem_PrintInfo(sem_key);
    sleep(3);
    
    printf("[%s:%d] sem key:%d\n",__FUNCTION__, __LINE__,sem_key);    
    if(sem_key >= 0)
    {   
        printf("[%s:%d] Try Lock Time:10s\n",__FUNCTION__, __LINE__);        
        if( Sem_TimedRdLock(sem_key, sec(10)))
        {            
            printf("[%s:%d] Get Lock, sleep:2s\n",__FUNCTION__, __LINE__);
            sleep(2);            
            ret = Sem_RdUnLock(sem_key);            
            printf("[%s:%d] UnLock, ret:%d\n",__FUNCTION__, __LINE__, ret);
        }
        else
        {
            printf("[%s:%d] Lock Failed\n",__FUNCTION__, __LINE__);
        }
    }  

}

void WlockTest1(void* argInfo)
{
    int sem_key = -1;
    bool ret = false;
    sem_key =Sem_RWLockInit(RWLOCK_PATH);  
    //Sem_PrintInfo(sem_key);
    sleep(2);
    
    printf("[%s:%d] sem key:%d\n",__FUNCTION__, __LINE__,sem_key);    
    if(sem_key >= 0)
    {   
        printf("[%s:%d] Try Lock Time:5s\n",__FUNCTION__, __LINE__);        
        if( Sem_TimedWrLock(sem_key, sec(5)))
        {            
            printf("[%s:%d] Get Lock, sleep:2s\n",__FUNCTION__, __LINE__);
            sleep(2);       
            ret = Sem_WrUnLock(sem_key);            
            printf("[%s:%d] UnLock, ret:%d\n",__FUNCTION__, __LINE__, ret);
        }
        else
        {
            printf("[%s:%d] Lock Failed\n",__FUNCTION__, __LINE__);
        }
    }  

}

void WlockTest2(void* argInfo)
{
    int sem_key = -1;
    bool ret = false;
    sem_key =Sem_RWLockInit(RWLOCK_PATH);   
    //Sem_PrintInfo(sem_key);
    sleep(3);
    
    printf("[%s:%d] sem key:%d\n",__FUNCTION__, __LINE__,sem_key);    
    if(sem_key >= 0)
    {   
        printf("[%s:%d] Try Lock Time:5s\n",__FUNCTION__, __LINE__);        
        if( Sem_TimedWrLock(sem_key, sec(5)))
        {            
            printf("[%s:%d] Get Lock, sleep:1s\n",__FUNCTION__, __LINE__);
            sleep(1);            
            ret = Sem_WrUnLock(sem_key);            
            printf("[%s:%d] UnLock, ret:%d\n",__FUNCTION__, __LINE__, ret);
        }
        else
        {
            printf("[%s:%d] Lock Failed\n",__FUNCTION__, __LINE__);
        }
    }   
}

void WlockTest3(void* argInfo)
{
    int sem_key = -1;
    bool ret = false;
    sem_key =Sem_RWLockInit(RWLOCK_PATH);  
    //Sem_PrintInfo(sem_key);
    sleep(4);
    
    printf("[%s:%d] sem key:%d\n",__FUNCTION__, __LINE__,sem_key);    
    if(sem_key >= 0)
    {   
        printf("[%s:%d] Try Lock Time:5s\n",__FUNCTION__, __LINE__);        
        if( Sem_TimedWrLock(sem_key, sec(5)))
        {            
            printf("[%s:%d] Get Lock, sleep:1s\n",__FUNCTION__, __LINE__);
            sleep(1);            
            ret = Sem_WrUnLock(sem_key);            
            printf("[%s:%d] UnLock, ret:%d\n",__FUNCTION__, __LINE__, ret);
        }
        else
        {
            printf("[%s:%d] Lock Failed\n",__FUNCTION__, __LINE__);
        }
    }  
}


bool Sem_RWLockTest()
{   
    static pthread_t a_thread = (pthread_t) 0;
    pthread_attr_t sPthreadAttr;
    int i32Ret = -1;
#if 1
    if (pthread_attr_init(&sPthreadAttr) == 0)
    {
        if (pthread_attr_setdetachstate(&sPthreadAttr, PTHREAD_CREATE_DETACHED) == 0)
        {
            if ((i32Ret = pthread_create(&a_thread, 
                                        &sPthreadAttr, 
                                        (void *(*)(void *)) RlockTest1, 
                                        (void *) NULL)) != 0)
            {                
                printf("%s() - Failed to create thread...", __FUNCTION__);                
                return false;
            }
        }    
    }

    if (pthread_attr_init(&sPthreadAttr) == 0)
    {
        if (pthread_attr_setdetachstate(&sPthreadAttr, PTHREAD_CREATE_DETACHED) == 0)
        {
            if ((i32Ret = pthread_create(&a_thread, 
                                        &sPthreadAttr, 
                                        (void *(*)(void *)) RlockTest2, 
                                        (void *) NULL)) != 0)
            {                
                printf("%s() - Failed to create thread...", __FUNCTION__);                
                return false;
            }
        }    
    }

    if (pthread_attr_init(&sPthreadAttr) == 0)
    {
        if (pthread_attr_setdetachstate(&sPthreadAttr, PTHREAD_CREATE_DETACHED) == 0)
        {
            if ((i32Ret = pthread_create(&a_thread, 
                                        &sPthreadAttr, 
                                        (void *(*)(void *)) RlockTest3, 
                                        (void *) NULL)) != 0)
            {                
                printf("%s() - Failed to create thread...", __FUNCTION__);                
                return false;
            }
        }    
    }
#endif
#if 1
    if (pthread_attr_init(&sPthreadAttr) == 0)
    {
        if (pthread_attr_setdetachstate(&sPthreadAttr, PTHREAD_CREATE_DETACHED) == 0)
        {
            if ((i32Ret = pthread_create(&a_thread, 
                                        &sPthreadAttr, 
                                        (void *(*)(void *)) WlockTest1, 
                                        (void *) NULL)) != 0)
            {                
                printf("%s() - Failed to create thread...", __FUNCTION__);                
                return false;
            }
        }    
    }

    if (pthread_attr_init(&sPthreadAttr) == 0)
    {
        if (pthread_attr_setdetachstate(&sPthreadAttr, PTHREAD_CREATE_DETACHED) == 0)
        {
            if ((i32Ret = pthread_create(&a_thread, 
                                        &sPthreadAttr, 
                                        (void *(*)(void *)) WlockTest2, 
                                        (void *) NULL)) != 0)
            {                
                printf("%s() - Failed to create thread...", __FUNCTION__);                
                return false;
            }
        }    
    }

    if (pthread_attr_init(&sPthreadAttr) == 0)
    {
        if (pthread_attr_setdetachstate(&sPthreadAttr, PTHREAD_CREATE_DETACHED) == 0)
        {
            if ((i32Ret = pthread_create(&a_thread, 
                                        &sPthreadAttr, 
                                        (void *(*)(void *)) WlockTest3, 
                                        (void *) NULL)) != 0)
            {                
                printf("%s() - Failed to create thread...", __FUNCTION__);                
                return false;
            }
        }    
    }
#endif
  
    return true;
}    

int main(int argc, char **argv)
{
    printf("--- Create two threads for semaphore test ---\n");
    Sem_CommonTest();
    sleep(40);

    printf("\n\n--- Create three semaphore RdLock and three semaphore WrLock to test ---\n");
    Sem_RWLockTest();    
    sleep(20);
    
    return 0;
}



