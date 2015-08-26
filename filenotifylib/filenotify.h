
#ifndef __FILENOTIFY_H__
#define __FILENOTIFY_H__

#include <sys/inotify.h>
#include <pthread.h>
#include "avct_list.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>


#define FILENOTIFY_MAX_PATH              256
#define FILENOTIFY_MAX_EVENT_BUFSIZE     4096

/* sysnotify return status */
#define FILENOTIFY_OK                       0
#define FILENOTIFY_MALLOC_FAILED            -1
#define FILENOTIFY_NULL_DATA                -2
#define FILENOTIFY_SYSTEM_ERROR             -3
#define FILENOTIFY_PARAMETER_ERROR          -4

/* sysnotify entry status, end: end of watch thread; running: watch thread is running */
enum SYSNOTIFY_ENTRY_STATUS{
    FILENOTIFY_ENTRY_STOP = 0x00,
    FILENOTIFY_ENTRY_RUNNING = 0x01
};

struct sFileNotify_watch_ops
{
    void (*in_access)(struct inotify_event *, char *);
    void (*in_attribute)(struct inotify_event *, char *);    
    void (*in_close_nowr)(struct inotify_event *, char *);   
    void (*in_close_wr)(struct inotify_event *, char *);
    void (*in_create)(struct inotify_event *, char *);
    void (*in_del)(struct inotify_event *, char *);
    void (*in_del_self)(struct inotify_event *, char *);
    void (*in_ignored)(struct inotify_event *, char *);
    void (*in_isdir)(struct inotify_event *, char *);
    void (*in_modify)(struct inotify_event *, char *);
    void (*in_move_self)(struct inotify_event *, char *);
    void (*in_move_from)(struct inotify_event *, char *);
    void (*in_move_to)(struct inotify_event *, char *);    
    void (*in_open)(struct inotify_event *, char *);    
    void (*in_unmount)(struct inotify_event *, char *);    
};


typedef struct sFileEventListTag
{
    struct list_head list;                      
    struct inotify_event *e;
} sFileEventList;

typedef struct sFileNotifyWatchEntryTag
{
    char path[FILENOTIFY_MAX_PATH];
    int status;
    unsigned long mask;
    int sysnotify_wd;
    int sysnotify_fd;
    struct sFileNotify_watch_ops *ops;
}sFileNotifyWatchEntryList;


/* External APIs */
int FileNotify_RegisterWatch(sFileNotifyWatchEntryList *new_entry ,int entry_count);
bool FileNotify_StopWatch(char *path);
void FileNotify_RemoveRegisterWatch();


#endif
