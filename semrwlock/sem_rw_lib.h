
#ifndef __SEMREADWRITE_H__
#define __SEMREADWRITE_H__

#include <pthread.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>


#define sec(x)      (1000 * x)
#define msec(x)     (x)
#define FOREVER     (0xFFFFFFFF)

enum eumnSemType
{
    SEM_WRLOCK  =0,
    SEM_RDLOCK  =1
};

int     Sem_RWLockInit(const char *pathname);
/*pair for write lock and unlock for only one process to access.*/
bool    Sem_TimedWrLock(int semid, unsigned int msTimeout);
bool    Sem_WrUnLock(int semid);
/*pair for read lock and unlock, these are 10 of the most process to access.*/
bool    Sem_TimedRdLock(int semid, unsigned int msTimeout);
bool    Sem_RdUnLock(int semid);


int     Sem_LockInit(const char *pathname);
/*pair for lock and unlock*/
bool    Sem_TimedLock(int semid, unsigned int msTimeout);
bool    Sem_UnLock(int semid);


void    Sem_Remove(int semid);
void    Sem_PrintInfo(int semid);

#endif
