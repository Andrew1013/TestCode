/******************************************************************************
* FILE: hello_mutex.c
* DESCRIPTION:
*   testing mutex and deadlock
******************************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS	5
#define NUM_TEST	5

pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

void taskA(void *threadid)
{    
   int i;   
   printf("task A! It's me, thread #%ld \n", pthread_self());

   for(i = 0; i < NUM_TEST; i++) {
	   pthread_mutex_lock(&mutex1);
	   sleep(1);
	   pthread_mutex_lock(&mutex2);
	   printf("task A in loopi %d \n",i );
	   sleep(1);
	   pthread_mutex_unlock(&mutex2);
	   sleep(1);
	   pthread_mutex_unlock(&mutex1);
	   sleep(1);
   }
   printf("task A! thread exit!\n");
   pthread_exit(NULL);
}

void taskB(void *threadid)
{   
   int i;   
   printf("task B! It's me, thread #%ld \n", pthread_self());

   for(i = 0; i < NUM_TEST; i++) {
	   pthread_mutex_lock(&mutex2);
	   sleep(1);
	   pthread_mutex_lock(&mutex1);
	   printf("task B in loopi %d \n",i );
	   sleep(1);
	   pthread_mutex_unlock(&mutex1);
	   sleep(1);
	   pthread_mutex_unlock(&mutex2);
	   sleep(1);
   }   
   printf("task B! thread exit!\n");
   pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    pthread_t a_thread = (pthread_t) 0;
    pthread_t b_thread = (pthread_t) 0;
    int rc, rc1;
    
    rc = pthread_create(&a_thread, 
                        NULL, 
                        (void *(*)(void *)) taskA, 
                        (void *) NULL);

    rc1 = pthread_create(&b_thread, 
                         NULL, 
                         (void *(*)(void *)) taskB, 
                         (void *) NULL);

    if(rc == 0)
        pthread_join(a_thread, NULL);
    if(rc1 == 0)
        pthread_join(b_thread, NULL);

    /* Last thing that main() should do */
    exit(-1);
}
