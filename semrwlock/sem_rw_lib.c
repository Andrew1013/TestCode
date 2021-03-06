#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include "sem_rw_lib.h"
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>


#define READ_LOCK_COUNT         10  
#define WRITE_LOCK_COUNT        1  
#define msleep(d)               usleep(1000 * d)

union semun {                   /* Used in calls to semctl() */
    int                 val;
    struct semid_ds     *buf;
    unsigned short      *array;
#if defined(__linux__)
    struct seminfo      *__buf;
#endif
};

enum eumnSemType
{
    SEM_WRLOCK  = 0,
    SEM_RDLOCK  = 1,   
    SEM_PRE_WRLOCK = 2,
    SEM_PRE_RDLOCK = 3,
};

enum eumnSemInitType
{
    COMMONLOCK_INIT  = 0,
    RWLOCK_INIT  = 1
};


int  semtimedop(int  semid, struct sembuf *sops, unsigned nsops, struct timespec *timeout);
static int GetLockValue(int semid, enum eumnSemType semType);
bool LockCheck(int semid, enum eumnSemType semType);
static int _Sem_Init(const char *pathname, int type);
static void Sem_DebugInfo(int semid, const char *szFunName);
static bool Sem_SetSemValue(int semid, enum eumnSemType semType, int value);
static int Sem_PrccessWaitCount(int semid, enum eumnSemType semType);

static int mstime_diff(struct timeval x , struct timeval y)
{
    double x_ms , y_ms , diff;
     
    x_ms = (double)x.tv_sec*1000000 + (double)x.tv_usec;
    y_ms = (double)y.tv_sec*1000000 + (double)y.tv_usec;
     
    diff = (double)y_ms - (double)x_ms;     
    return (int) diff/1000;
}

static key_t jenkins_key(const char *pathname)
{    
    unsigned int hash, i;   
    unsigned int len = strlen(pathname);
    for(hash = i = 0; i < len; ++i)    
    {        
        hash += pathname[i];        
        hash += (hash << 10);        
        hash ^= (hash >> 6);    
    }    
    hash += (hash << 3);    
    hash ^= (hash >> 11);    
    hash += (hash << 15);    
    return hash;
}

static bool Sem_PreLockWait(int semid,unsigned int msTimeout, enum eumnSemType semType)
{
    struct sembuf sem_b;
    struct timespec ts;
    int ret = 0;

    if(semType == SEM_WRLOCK ||  semType == SEM_RDLOCK)
        return false;    

    /* Lock The Resource */
    sem_b.sem_num = semType;
    sem_b.sem_op = -1; 
    sem_b.sem_flg = SEM_UNDO;

    if(msTimeout == FOREVER)
    {
        ret = semtimedop(semid, &sem_b , 1, NULL);
    }
    else
    {
        ts.tv_sec = (msTimeout / 1000);
        ts.tv_nsec = ((msTimeout % 1000) * 1000000);        
        ret = semtimedop(semid, &sem_b , 1, &ts);
    }
     
    if(ret < 0)
    {
        //Sem_DebugInfo(semid, __FUNCTION__);
        return false;
    }
    return true;
}

void Sem_Remove(int semid)
{
    union semun dummy;
    semctl( semid, 0, IPC_RMID, dummy );
}

int Sem_LockInit(const char *pathname)
{
    return _Sem_Init(pathname, COMMONLOCK_INIT);
}
int Sem_RWLockInit(const char *pathname)
{
    return _Sem_Init(pathname, RWLOCK_INIT);
}

static int _Sem_Init(const char *pathname, int type)
{
    int     semid, key;
    key = jenkins_key(pathname);      
     
    if(type == COMMONLOCK_INIT)
        semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666 );   
    else
        semid = semget(key, 4, IPC_CREAT | IPC_EXCL | 0666 );  
    
    if (semid != -1) 
    {   /* Successfully created the semaphore */
        union semun arg;
        struct sembuf sop[4];
        arg.val = 0;                    /* So initialize it to 0 */
        
        if(type == COMMONLOCK_INIT)
        {            
            if (semctl(semid, COMMONLOCK_INIT, SETVAL, arg) == -1)
            {
                syslog(LOG_USER | LOG_WARNING ,"%s semctl for COMMONLOCK_INIT failed\n", __FUNCTION__);
                return -1;
            }  
            sop[COMMONLOCK_INIT].sem_num = 0;                /* Operate on semaphore 0 */
            sop[COMMONLOCK_INIT].sem_op = 1;                 //for write
            sop[COMMONLOCK_INIT].sem_flg = 0;
            if (semop(semid, sop, 1) == -1)
            {
                syslog(LOG_USER | LOG_WARNING ,"%s semop failed\n", __FUNCTION__);
                return -1;
            }           
        }
        else
        {
            if (semctl(semid, SEM_WRLOCK, SETVAL, arg) == -1)
            {
                syslog(LOG_USER | LOG_WARNING ,"%s semctl for SEM_WRITE failed\n", __FUNCTION__);
                return -1;
            }          
            if (semctl(semid, SEM_RDLOCK, SETVAL, arg) == -1)
            {
                syslog(LOG_USER | LOG_WARNING ,"%s semctl for SEM_READ failed\n", __FUNCTION__);
                return -1;
            } 
            if (semctl(semid, SEM_PRE_WRLOCK, SETVAL, arg) == -1)
            {
                syslog(LOG_USER | LOG_WARNING ,"%s semctl for SEM_WRITE failed\n", __FUNCTION__);
                return -1;
            }          
            if (semctl(semid, SEM_PRE_RDLOCK, SETVAL, arg) == -1)
            {
                syslog(LOG_USER | LOG_WARNING ,"%s semctl for SEM_READ failed\n", __FUNCTION__);
                return -1;
            }                

            sop[SEM_WRLOCK].sem_num = 0;                /* Operate on semaphore 0 */
            sop[SEM_WRLOCK].sem_op = WRITE_LOCK_COUNT;                 //for write
            sop[SEM_WRLOCK].sem_flg = 0;
            sop[SEM_RDLOCK].sem_num = 1;                /* Operate on semaphore 1 */
            sop[SEM_RDLOCK].sem_op = READ_LOCK_COUNT;                 /* Wait for value to equal 0 */
            sop[SEM_RDLOCK].sem_flg = 0;    
                
            if (semop(semid, sop, 2) == -1)
            {
                syslog(LOG_USER | LOG_WARNING ,"%s semop failed\n", __FUNCTION__);
                return -1;
            }            
        }        

    } 
    else 
    {                            /* We didn't create the semaphore set */

        if (errno != EEXIST) 
        {   /* Unexpected error from semget() */
            syslog(LOG_USER | LOG_WARNING ,"%s Unexpected error from semget\n", __FUNCTION__);           
        } 
        else 
        { 
            /* Someone else already created it */
            const int MAX_TRIES = 10;
            int j;
            union semun arg;
            struct semid_ds ds;            
            semid = semget(key, 0, 0);      /* So just get ID */
            if (semid == -1)
            {
                syslog(LOG_USER | LOG_WARNING ,"%s semget failed\n", __FUNCTION__);
                return -1;
            }                
            
            /* Wait until another process has called semop() */
            arg.buf = &ds;
            for (j = 0; j < MAX_TRIES; j++) 
            {
                if (semctl(semid, 0, IPC_STAT, arg) == -1)
                {
                    syslog(LOG_USER | LOG_WARNING ,"%s semctl failed\n", __FUNCTION__);
                    return -1;
                }     
                
                if (ds.sem_otime != 0)          /* Semop() performed? */
                    break;                      /* Yes, quit loop */
                sleep(1);                       /* If not, wait and retry */
            }            
        }
    }
    syslog(LOG_USER | LOG_INFO ,"%s  semaphore initialized, semid:%d, key:%x, path:%s\n\n", __FUNCTION__,semid, key, pathname);
    return semid;

}

bool Sem_TimedLock(int semid, unsigned int msTimeout)
{
    struct sembuf sem_b;
    struct timespec ts;
    int ret = 0;

    /* Lock The Resource */
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; 
    sem_b.sem_flg = SEM_UNDO;

    if(msTimeout == FOREVER)
    {
        ret = semtimedop(semid, &sem_b , 1, NULL);
    }
    else
    {
        ts.tv_sec = (msTimeout / 1000);
        ts.tv_nsec = ((msTimeout % 1000) * 1000000);        
        ret = semtimedop(semid, &sem_b , 1, &ts);
    }
     
    if(ret < 0)
    {
        Sem_DebugInfo(semid, __FUNCTION__);
        return false;
    }
    return true;
}

bool Sem_UnLock(int semid)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = 1;  //Decress
    sem_b.sem_flg = SEM_UNDO /* | IPC_NOWAIT */;
    if(semop(semid, &sem_b, 1) == -1) 
    {
        Sem_DebugInfo(semid, __FUNCTION__);
        return false;
    }
    return true;
}

bool Sem_TimedRdLock(int semid,unsigned int u32msTimeout)
{
    struct sembuf sem_b;    
    struct timespec ts;
    int ret = 0;
    long long msTimeout = u32msTimeout;
    struct timeval before , after;

    gettimeofday(&before , NULL); 
    if(LockCheck(semid, SEM_WRLOCK) || 
      (Sem_PrccessWaitCount(semid, SEM_PRE_WRLOCK) > 0 ))    
    { 
        if(Sem_PreLockWait(semid, u32msTimeout, SEM_PRE_RDLOCK) == false)
        { 
            Sem_DebugInfo(semid, __FUNCTION__);
            return false;
        }    
    }
    Sem_SetSemValue(semid, SEM_PRE_WRLOCK, 0);
    gettimeofday(&after , NULL);
    msTimeout -= mstime_diff(before , after) ;      
    
    /* Lock The Resource */
    if(msTimeout > 0)
    {
        sem_b.sem_num = SEM_RDLOCK;//SEM_WRLOCK;
        sem_b.sem_op = -1; 
        sem_b.sem_flg = SEM_UNDO;

        //ret = semop(semid, &sem_b, 1);
        if(msTimeout == FOREVER)
        {
            ret = semtimedop(semid, &sem_b , 1, NULL);
        }
        else
        {
            ts.tv_sec = (msTimeout / 1000);
            ts.tv_nsec = ((msTimeout % 1000) * 1000000);        
            ret = semtimedop(semid, &sem_b , 1, &ts);
        }
    }
    else
    {
        Sem_DebugInfo(semid, __FUNCTION__);
        return false;
    }   
  
    if(ret < 0)
    {
        Sem_DebugInfo(semid, __FUNCTION__);
        return false;
    }
    return true;    
}

bool Sem_RdUnLock(int semid)
{
    struct sembuf sem_b;    
    int ret = 0;

    /* Lock The Resource */
    sem_b.sem_num = SEM_RDLOCK;//SEM_WRLOCK;
    sem_b.sem_op = 1; 
    sem_b.sem_flg = SEM_UNDO;

    ret = semop(semid, &sem_b, 1);
    if(ret < 0)
    {
        return false;
    }

    if(GetLockValue(semid, SEM_RDLOCK) == READ_LOCK_COUNT && 
        GetLockValue(semid, SEM_PRE_WRLOCK) == 0 &&
        Sem_PrccessWaitCount(semid, SEM_PRE_WRLOCK) > 0)
    {
        Sem_SetSemValue(semid, SEM_PRE_WRLOCK, WRITE_LOCK_COUNT);   
    }
    
    return true;    
}

static int Sem_PrccessWaitCount(int semid, enum eumnSemType semType)
{
    union semun dummy;
    return semctl(semid, semType, GETNCNT, dummy);
}

//static int Sem_SetSemValue(int semid, enum eumnSemType semType, int value)
static bool Sem_SetSemValue(int semid, enum eumnSemType semType, int value)
{
    union semun arg;
    arg.val = value;   
    if (semctl(semid, semType, SETVAL, arg) == -1)
    {
        syslog(LOG_USER | LOG_WARNING ,"%s semctl for SEM_WRITE failed\n", __FUNCTION__);
        return false;
    } 
    return true;
}

bool Sem_TimedWrLock(int semid, unsigned int u32msTimeout)
{
    struct sembuf sem_b;    
    struct timespec ts;
    int ret = 0;
    struct timeval before , after;
    long long msTimeout = u32msTimeout;

    gettimeofday(&before , NULL);
    if(LockCheck(semid, SEM_WRLOCK) || GetLockValue(semid, SEM_RDLOCK) != READ_LOCK_COUNT)
    {        
        if(Sem_PreLockWait(semid, u32msTimeout, SEM_PRE_WRLOCK) == false)
        {          
            Sem_DebugInfo(semid, __FUNCTION__);
            return false;
        }   
    }
    Sem_SetSemValue(semid, SEM_PRE_RDLOCK, 0);
    gettimeofday(&after , NULL);
    msTimeout -= mstime_diff(before , after);      
        
    /* Lock The Resource */
    sem_b.sem_num = SEM_WRLOCK;//SEM_WRLOCK;
    sem_b.sem_op = -1; 
    sem_b.sem_flg = SEM_UNDO;
    
    //ret = semop(semid, &sem_b, 1);
    if(msTimeout > 0)
    {
        if(msTimeout == FOREVER)
        {
            ret = semtimedop(semid, &sem_b , 1, NULL);
        }
        else
        {
            ts.tv_sec = (msTimeout / 1000);
            ts.tv_nsec = ((msTimeout % 1000) * 1000000);        
            ret = semtimedop(semid, &sem_b , 1, &ts);
        }  
    }
    else
    {
        Sem_DebugInfo(semid, __FUNCTION__);
        return false;
    }
    
    if(ret < 0 )
    {
        Sem_DebugInfo(semid, __FUNCTION__);
        return false;        
    }    
    
    return true;     
}

bool Sem_WrUnLock(int semid)
{
    struct sembuf sem_b;    
    int ret = 0;

    /* Lock The Resource */
    sem_b.sem_num = SEM_WRLOCK;
    sem_b.sem_op = 1; 
    sem_b.sem_flg = SEM_UNDO;

    ret = semop(semid, &sem_b, 1);
    if(ret < 0)
    {
        return false;
    }
        
    if(Sem_PrccessWaitCount(semid, SEM_PRE_RDLOCK) > 0 && LockCheck(semid, SEM_WRLOCK) == false)
        Sem_SetSemValue(semid, SEM_PRE_RDLOCK, READ_LOCK_COUNT);   
    else if(Sem_PrccessWaitCount(semid, SEM_PRE_WRLOCK) > 0 && LockCheck(semid, SEM_WRLOCK) == false)
        Sem_SetSemValue(semid, SEM_PRE_WRLOCK, WRITE_LOCK_COUNT);   

    return true;    
}

bool LockCheck(int semid, enum eumnSemType semType)
{    
    struct sembuf sem_b;

    sem_b.sem_num = semType;
    sem_b.sem_op = 0;
    sem_b.sem_flg = SEM_UNDO | IPC_NOWAIT;

    if(semop(semid,&sem_b,1) == -1)
    {
        return false;
    }
    return true;

}

static int GetLockValue(int semid, enum eumnSemType semType)
{    
    union semun arg;            
    return semctl(semid,semType,GETVAL,arg);    
}

static void Sem_DebugInfo(int semid, const char *szFunName)
{
    struct semid_ds ds;
    union semun arg, dummy;             /* Fourth argument for semctl() */
    int j;

    arg.buf = &ds;
    if (semctl(semid, 0, IPC_STAT, arg) == -1)
    {
        syslog(LOG_USER | LOG_WARNING ,"%s semctl failed\n", __FUNCTION__);
        return;
    }  
    /* Display per-semaphore information */

    arg.array = alloca(ds.sem_nsems * sizeof(arg.array[0]));
    if (arg.array == NULL)
    {
        syslog(LOG_USER | LOG_WARNING ,"%s alloca failed\n", __FUNCTION__);
        return;
    }            
    if (semctl(semid, 0, GETALL, arg) == -1)
    {
        syslog(LOG_USER | LOG_WARNING ,"%s semctl GETALL failed\n", __FUNCTION__);
        return;
    }
    
    for (j = 0; j < ds.sem_nsems; j++)
        syslog(LOG_USER | LOG_WARNING ,"semkey:%d, fun:%s, num#:%d, value:%d, pid:%d, cnt:%d.", 
                semid,
                szFunName,
                j, 
                arg.array[j],
                semctl(semid, j, GETPID, dummy),
                semctl(semid, j, GETNCNT, dummy)
                );

}

void Sem_PrintInfo(int semid)
{
    struct semid_ds ds;
    union semun arg, dummy;             /* Fourth argument for semctl() */
    int j;

    arg.buf = &ds;
    if (semctl(semid, 0, IPC_STAT, arg) == -1)
    {
        syslog(LOG_USER | LOG_WARNING ,"%s semctl failed\n", __FUNCTION__);
        return;
    }    

    printf("\tSemaphore changed: %s", ctime(&ds.sem_ctime));
    printf("\tLast semop():      %s", ctime(&ds.sem_otime));

    /* Display per-semaphore information */

    arg.array = alloca(ds.sem_nsems * sizeof(arg.array[0]));
    if (arg.array == NULL)
    {
        syslog(LOG_USER | LOG_WARNING ,"%s alloca failed\n", __FUNCTION__);
        return;
    }            
    if (semctl(semid, 0, GETALL, arg) == -1)
    {
        syslog(LOG_USER | LOG_WARNING ,"%s semctl GETALL failed\n", __FUNCTION__);
        return;
    } 
    printf("\tSem #  Value  SEMPID  SEMNCNT  SEMZCNT\n");
    for (j = 0; j < ds.sem_nsems; j++)
        printf("\t%3d   %5d   %5d  %5d    %5d\n", j, arg.array[j],
                semctl(semid, j, GETPID, dummy),
                semctl(semid, j, GETNCNT, dummy),
                semctl(semid, j, GETZCNT, dummy));
    
}




