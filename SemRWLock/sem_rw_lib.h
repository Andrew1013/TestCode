
#ifndef __SEMREADWRITE_H__
#define __SEMREADWRITE_H__

#include <pthread.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>


#define sec(x)      (1000 * x)
#define ms(x)       (x)
#define FOREVER     (0xFFFFFFFF)

enum eumnSemType
{
    SEM_WRLOCK =0,
    SEM_RDLOCK= 1
};

union semun {                   /* Used in calls to semctl() */
    int                 val;
    struct semid_ds     *buf;
    unsigned short      *array;
#if defined(__linux__)
    struct seminfo      *__buf;
#endif
};

int Sem_RWLockInit(const char *pathname);
bool Sem_TimedWrLock(int semid, int msTimeout);
bool Sem_TimedRdLock(int semid, int msTimeout);
bool Sem_WrUnLock(int semid);
bool Sem_RdUnLock(int semid);


int Sem_LockInit(const char *pathname);
bool Sem_UnLock(int semid);
bool Sem_TimedLock(int semid, int msTimeout);


void Sem_Remove(int semid);
void Sem_PrintInfo(int semid);

#endif
