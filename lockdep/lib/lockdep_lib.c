/*
 * pthread lockdep
 *
 * modified code from http://ceph.com/dev-notes/lockdep-for-pthreads
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software
 * Foundation.  See file COPYING.
 *
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dlfcn.h>
#include <assert.h>
#include <unistd.h>

#define	MAXPID	40
#define	MAXLOCK	30
//#define DEBUG


#define ERR_PRINT(fmt, args...) { \
    printf("[%s](%d):Error: "fmt, __FUNCTION__, __LINE__, ##args); \
}
//#define INFO
#ifdef INFO 
#define INFO_PRINT(fmt, args...) { \
    printf("[%s](%d):Info: "fmt, __FUNCTION__, __LINE__, ##args); \
}
#else
    #define INFO_PRINT(fmt, args...) do { \
    }while(0) ; 
#endif
//#define DBG
#ifdef DBG
#define DBG_PRINT(fmt, args...) { \
    printf("[%s](%d):Info: "fmt, __FUNCTION__, __LINE__, ##args); \
}
#else
    #define DBG_PRINT(fmt, args...) do { \
    }while(0) ; 
#endif

typedef	struct	Btrace	Btrace;

struct	Btrace
{
	unsigned long	pid;
	int	deadlock;
	unsigned char	payload[1];
};

enum
{
	btsize	 = 600,
};

static	pthread_mutex_t lockdep_lk = PTHREAD_MUTEX_INITIALIZER;
static	pthread_mutex_t*	held[MAXPID][MAXLOCK];
static	pthread_mutex_t*	lockid[MAXLOCK];
static	pthread_t	pid_tab[MAXPID];
static	Btrace*		follows[MAXLOCK][MAXLOCK];
//static	int		detected = 0;
static	int		init = 0;


void _init_mutex()  __attribute__((constructor));
void  _fini_mutex()  __attribute__((destructor)); 


typedef int (*lp_pthread_mutex_lock)(pthread_mutex_t *mutex);
typedef int (*lp_pthread_mutex_unlock)(pthread_mutex_t *mutex);


static lp_pthread_mutex_lock  _pthread_lock_hook = NULL;
static lp_pthread_mutex_unlock  _pthread_unlock_hook = NULL;



static int hook_lock(lp_pthread_mutex_lock *fptr, const char *fname)
{
    char *msg = NULL;

    assert(fname != NULL);

    if (*fptr == NULL) {
        INFO_PRINT("dlsym : wrapping %s\n", fname);
        *fptr = dlsym(RTLD_NEXT, fname);
        INFO_PRINT("next_%s = %p\n", fname, *fptr);
        if ((*fptr == NULL) || ((msg = dlerror()) != NULL)) {
            ERR_PRINT("dlsym %s failed : %s\n", fname, msg);
            return -1;
        } 
        else {
            INFO_PRINT("dlsym: wrapping %s done\n", fname);
            return 0;
        }
    } else {
        return 0;
    }
}

static int hook_unlock(lp_pthread_mutex_unlock *fptr, const char *fname)
{
    char *msg = NULL;

    assert(fname != NULL);

    if (*fptr == NULL) {

        INFO_PRINT("dlsym : wrapping %s\n", fname);
        *fptr = dlsym(RTLD_NEXT, fname);
        if ((*fptr == NULL) || ((msg = dlerror()) != NULL)) {
            ERR_PRINT("dlsym %s failed : %s\n", fname, msg);
            return -1;
        } 
        else {
            INFO_PRINT("dlsym: wrapping %s done\n", fname);
            return 0;
        }
    } 
    else {
        return 0;
    }
}


static void hook_lockfuncs(void)
{
    if (_pthread_lock_hook == NULL) {
        int rc = hook_lock(&_pthread_lock_hook, "pthread_mutex_lock"); 
        if (NULL == _pthread_lock_hook || rc != 0) {
            ERR_PRINT("Failed to pthread_lock_hook.\n");
            exit(EXIT_FAILURE);
        }
    }
}

static void hook_unlockfuncs(void)
{
    if (_pthread_unlock_hook == NULL) {
        int rc = hook_unlock(&_pthread_unlock_hook, "pthread_mutex_unlock"); 
        if (NULL == _pthread_unlock_hook || rc != 0) {
            ERR_PRINT("Failed to pthread_unlock_hook.\n");
            exit(EXIT_FAILURE);
        }
    }
}


static void lock_lockdep(void)
{    
	_pthread_lock_hook(&lockdep_lk);
}

static void unlock_lockdep(void)
{
	_pthread_unlock_hook(&lockdep_lk);
}

static int get_lockid(pthread_mutex_t *q)
{
	int i, found;
	
	found = 0;
	for(i = 0; i < MAXLOCK && lockid[i] != NULL; i++) {
		if(lockid[i] == q) {
			found = 1;
			break;
		}
	}
	if(found == 0)
		lockid[i] = q;

	return i;
}


static pthread_mutex_t * get_lock(int i)
{
	if(i < 0 || i >=MAXLOCK)
		return NULL;

	return lockid[i];
}



static int get_internal_pid()
{
	int i, found;
	pthread_t tid;

	tid = pthread_self();


	INFO_PRINT("[%s](%d)tid: %lx pid:%d\r\n",__FUNCTION__,__LINE__,tid, getpid());

	found = 0;
	for(i = 0; i < MAXPID && pid_tab[i] != -1; i++) {
		if(pid_tab[i] == tid) {
			found = 1;
			break;
		}
	}
	if(found == 0)
		pid_tab[i] = tid;

	return i;
}

static pthread_t get_pid(int i)
{
	if(i < 0 || i >=MAXPID)
		return (pthread_t) 0;

	return pid_tab[i];
}

static int does_follow(int a, int b)
{
	int i;
	
	if(a < 0 || a >= MAXLOCK || b < 0 || b >= MAXLOCK)
		return 0;

	if(follows[a][b]){
		printf("[Track ID: %ld]A-->B  B-->A deadlock happen\r\n", (follows[b][a])->pid);
		return 1;
	}	

	for(i=0; i<MAXLOCK; i++) {
		if(follows[a][i] && does_follow(i, b)) {
            printf("[Track ID: %ld]A-->B  B-->C  C-->A circuit deadlock happen\r\n", (follows[b][a])->pid);
			printf("existing intermediate dependency \n");		
			return 1;
		}
	}

	return 0;
}

static void lockdep_init(void)
{
	int i, j;


	DBG_PRINT("lockdep_init <<< \n");

	if(init == 0) {
		for(i = 0; i < MAXLOCK; i++)
			for(j = 0; j < MAXLOCK; j++)
				follows[i][j] = NULL;

		for(i = 0; i < MAXLOCK; i++)
			lockid[i] = NULL;

		for(i = 0; i < MAXPID; i++)
			pid_tab[i] = -1;

		for(i = 0; i < MAXPID; i++)
			for(j = 0; j < MAXLOCK; j++)
				held[i][j] = NULL;

		init = 1;
	}


	DBG_PRINT("lockdep_init >>> \n");


}


void dump_lockdep(int dmpbt)
{
	int i, j;
	Btrace	*bt;
	pthread_mutex_t	*qlk;
  
	printf("=== current mutex with holding threads (TID) and process (PID) ===\n");

	for(i = 0; i < MAXPID; i++)
		for(j = 0; j < MAXLOCK; j++)
			if(held[i][j] != NULL) {
				qlk = get_lock(j);
				printf("[Track ID: %x](pid: %lu)(tid: %lu) holds lock 0x%lu (id=%d)\n",
					i,  (unsigned long int)getpid(), pid_tab[i], (unsigned long int) qlk, j);
			}

	printf("\n");
	

	printf("=== Recorded lock dependency ===\n");
	for(i = 0; i < MAXLOCK; i++)
		for(j = 0; j < MAXLOCK; j++) {
			bt = follows[i][j];
			if(bt != NULL) {
				printf("[Track ID: %lu](pid: %lu)(tid: %lu) lock 0x%p (id=%d) -> lock 0x%p (id=%d) %s\n",
					 (unsigned long int)bt->pid,  (unsigned long int) getpid(), pid_tab[bt->pid],get_lock(i), i, get_lock(j), j,
					bt->deadlock==1?"deadlock":"ok");
				//if(dmpbt == 1)
					//printf("%s\n\n", (char *) bt->payload);
			}
		}

}

static int will_lock(pthread_mutex_t *mutex, int pid)
{
	int	llockid, i;
	Btrace	*bt;

	if(pid < 0 || pid >= MAXPID || mutex == NULL)
		return -1;

	lock_lockdep();


	DBG_PRINT("will_lock >>>\n");


	llockid = get_lockid(mutex);

	for(i = 0; i < MAXLOCK; i++) {
		if(held[pid][i] == NULL)
			continue;

		if(!follows[i][llockid]) {
			bt = (Btrace*)malloc((sizeof(Btrace) + btsize));
			//backtrace(bt->payload, btsize, &sp);
			if(bt == NULL)
				ERR_PRINT("will_lock: *** malloc error \n");

			follows[i][llockid] = bt;
			(follows[i][llockid])->pid = pid;

			if(does_follow(llockid, i)) {

				//if(detected == 0) {
					printf("\r\n");
					printf("=== **** lock detected **** ===\r\n");
					printf("[tid 0x%lx] lock %lux (id=%d) -> lock %lux (id=%d)\n",
						get_pid(pid), (unsigned long)get_lock(i), i, (unsigned long)mutex, llockid);
					printf("\r\n");
					//detected = 1;
					(follows[i][llockid])->deadlock = 1;

					dump_lockdep(1);
				//}

				break;
			}
			else
			{
				(follows[i][llockid])->pid = pid;
				//printf("%t will_lock: pid %lud lock 0x%lux (id=%d) -> lock 0x%lux (id=%d, %s)\n",
					//pid, (unsigned long)get_lock(i), i, (unsigned long)mutex, lockid, q->name);
			}
		}
	}
	DBG_PRINT("will_lock <<<\n");

	unlock_lockdep();
	return 0;
}



static int locked(pthread_mutex_t *mutex, int pid)
{
	int	llockid;


	DBG_PRINT("locked >>> \n");


	if(pid < 0 || pid >= MAXPID || mutex == NULL)
		return -1;

	lock_lockdep();

	llockid = get_lockid(mutex);
	held[pid][llockid] = mutex;

	unlock_lockdep();


	DBG_PRINT("locked <<< \n");


	return 0;
}

static int unlocked(pthread_mutex_t *mutex, int pid)
{

	int	llockid;


	DBG_PRINT("unlocked >>>\n");


	if(pid < 0 || pid >= MAXPID || mutex == NULL)
		return -1;

	lock_lockdep();

	llockid = get_lockid(mutex);
	held[pid][llockid] = NULL;

	unlock_lockdep();


	DBG_PRINT("unlocked <<<\n");


	return 0;
}

int mutex_lock(pthread_mutex_t *mutex)
{
	int r, pid;
      
	if(init == 0)
		lockdep_init();

	pid = get_internal_pid();
	will_lock(mutex, pid);
	r = _pthread_lock_hook(mutex);
	locked(mutex, pid);
	return r;
}

int mutex_unlock(pthread_mutex_t *mutex)
{
	int r, pid;
    
	if(init == 0)
		lockdep_init();

	pid = get_internal_pid();
	r = _pthread_unlock_hook(mutex);
	unlocked(mutex, pid);
	return r;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    return mutex_lock(mutex);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    return mutex_unlock(mutex);
}

void _init_mutex()
{
  
    INFO_PRINT("*** _init().\n");
    hook_lockfuncs();
    hook_unlockfuncs();
}


void  _fini_mutex()
{
    INFO_PRINT("*** _fini().\n");
}


