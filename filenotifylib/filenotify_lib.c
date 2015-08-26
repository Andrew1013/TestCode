#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>
#include <errno.h>
#include "filenotify.h"

#define msleep(x) usleep(x*1000)

sFileNotifyWatchEntryList *G_FileWatchEntry;
static sFileEventList S_FileEventList;   
static unsigned char S_FileEventCount = 0;   

pthread_mutex_t sysnotify_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t sysnotify_cond = PTHREAD_COND_INITIALIZER;

static pthread_t EventRead_thread = 0;
static pthread_t EventHandler_thread = 0;


static int FileNotify_PopEvent(struct inotify_event **pNoityEvent);
static int FileNotify_PushEvent(struct inotify_event *pNoityEvent);
static int FileNotify_CheckEvent(int fd);
//static int FileNotify_AddWatchEntry(sFileNotifyWatchEntryList *psFileWatchEntry);
static bool FileNotify_Open(int *fd);
static bool FileNotify_Cose(int fd);
static int FileNotify_RemoveWatch(int fd, int wd);
static int FileNotify_ReadEevent(int fd );

bool FileNotify_StopWatch(char *path)
{    
    int i = 0;

    for(i =0 ; i<S_FileEventCount; i++ )
    {
        if(strcmp(G_FileWatchEntry[i].path, path)== 0)
        {
            FileNotify_RemoveWatch(G_FileWatchEntry[i].sysnotify_fd, G_FileWatchEntry[i].sysnotify_wd);
            G_FileWatchEntry[i].status = FILENOTIFY_ENTRY_STOP;
            G_FileWatchEntry[i].sysnotify_wd = -1;     
            return true;
        }
    } 
    return false;
}

void FileNotify_RemoveRegisterWatch()
{
    int i = 0;
    struct list_head *pos, *next;    
    
    for(i =0 ; i<S_FileEventCount; i++ )
    {
        if(G_FileWatchEntry[i].status == FILENOTIFY_ENTRY_RUNNING)
        {
            FileNotify_RemoveWatch(G_FileWatchEntry[i].sysnotify_fd, G_FileWatchEntry[i].sysnotify_wd);
            G_FileWatchEntry[i].status = FILENOTIFY_ENTRY_STOP;
            G_FileWatchEntry[i].sysnotify_wd = -1;            
        }
    } 
    FileNotify_Cose(G_FileWatchEntry[i].sysnotify_fd);
     
    avct_list_for_each_safe(pos, next, &S_FileEventList.list)
    {
        pthread_mutex_lock(&sysnotify_mutex);
        sFileEventList *psFileEvent = NULL;       
        psFileEvent = avct_list_entry(pos, sFileEventList, list);            
        avct_list_del(pos, &S_FileEventList.list);        
        free(psFileEvent->e); 
        free(psFileEvent);  
        pthread_mutex_unlock(&sysnotify_mutex); 
    }
    free(G_FileWatchEntry);
    pthread_cond_destroy(&sysnotify_cond);    
    pthread_cancel(EventRead_thread);
    pthread_cancel(EventHandler_thread);
    
}


static int FileNotify_AddWatch(int fd, const char *path, unsigned long mask)
{
    int wd = 0;

    wd = inotify_add_watch(fd, path, mask);
    if (wd < 0)
    {
        perror("inotify_add_watch");
        syslog( LOG_USER|LOG_ERR, "pid=%d, %s - inotify_add_watch error.\n", getpid(), __FUNCTION__);
    }
    return wd;
}


void *FlieNotify_EventRead(void *arg)
{
    int sysnotify_fd = 0, sysnotify_wd = 0;
    bool bRet = false;
    int i = 0;
    bRet = FileNotify_Open(&sysnotify_fd);
    if(bRet == false )
    {       
        syslog( LOG_USER|LOG_ERR, "pid=%d, %s - Invalid file descriptor(%m).\n", getpid(), __FUNCTION__);  
    }

    for(i =0 ; i<S_FileEventCount; i++ )
    {
        sysnotify_wd = FileNotify_AddWatch(sysnotify_fd, G_FileWatchEntry[i].path, G_FileWatchEntry[i].mask);
        if(sysnotify_wd < 0)
        {            
            syslog( LOG_USER|LOG_ERR, "pid=%d, %s - Invalid watch descriptor(%m), watched path %s.\n", 
            getpid(), __FUNCTION__, G_FileWatchEntry[i].path);
            G_FileWatchEntry[i].sysnotify_fd = sysnotify_fd;
            G_FileWatchEntry[i].sysnotify_wd = -1;                
            G_FileWatchEntry[i].status = FILENOTIFY_ENTRY_STOP;
        }
        else
        {
            G_FileWatchEntry[i].sysnotify_fd = sysnotify_fd;
            G_FileWatchEntry[i].sysnotify_wd = sysnotify_wd;    
            G_FileWatchEntry[i].status = FILENOTIFY_ENTRY_RUNNING;
        }
    }     

    while(1)
    {
        pthread_testcancel();
        if(FileNotify_CheckEvent(sysnotify_fd) > 0)
        {
            pthread_testcancel();
            FileNotify_ReadEevent(sysnotify_fd);
        }
    }
}

void *FlieNotify_EventHandler(void *arg)
{
    struct inotify_event *event_entry = NULL;
    sFileNotifyWatchEntryList *watch_entry = NULL;
    int i = 0;
     
    while(1)
    {        
        pthread_mutex_lock(&sysnotify_mutex);        
        while(FileNotify_PopEvent(&event_entry) != FILENOTIFY_OK)
        {     
            pthread_testcancel();
            pthread_cond_wait(&sysnotify_cond,&sysnotify_mutex);
        }
        pthread_mutex_unlock(&sysnotify_mutex); 
        
        if(event_entry)            
        {
            for(i =0 ; i<S_FileEventCount; i++ )
            {
                if(G_FileWatchEntry[i].sysnotify_wd == event_entry->wd)
                {
                    watch_entry = &G_FileWatchEntry[i];
                    break;
                }
            }
        }        
        
        if(watch_entry != NULL && watch_entry->status == FILENOTIFY_ENTRY_RUNNING)
        {
            switch (event_entry->mask)
            {
                case IN_ACCESS:
                if(watch_entry->ops->in_access)
                watch_entry->ops->in_access(event_entry, watch_entry->path);
                break;

                case IN_ATTRIB:
                if(watch_entry->ops->in_attribute)
                watch_entry->ops->in_attribute(event_entry, watch_entry->path);
                break;


                case IN_CLOSE_NOWRITE:
                if(watch_entry->ops->in_close_nowr)
                watch_entry->ops->in_close_nowr(event_entry, watch_entry->path);
                break;

                case IN_CLOSE_WRITE:
                if(watch_entry->ops->in_close_wr)
                watch_entry->ops->in_close_wr(event_entry, watch_entry->path);
                break;

                case IN_CREATE:
                if(watch_entry->ops->in_create)
                watch_entry->ops->in_create(event_entry, watch_entry->path);
                break;

                case IN_DELETE:
                if(watch_entry->ops->in_del)
                watch_entry->ops->in_del(event_entry, watch_entry->path);
                break;  

                case IN_DELETE_SELF:
                if(watch_entry->ops->in_del_self)
                watch_entry->ops->in_del_self(event_entry, watch_entry->path);
                break;

                case IN_IGNORED:
                 if(watch_entry->ops->in_ignored)
                 watch_entry->ops->in_ignored(event_entry, watch_entry->path);
                 break;

                case IN_ISDIR:
                if(watch_entry->ops->in_isdir)
                watch_entry->ops->in_open(event_entry, watch_entry->path);
                break;

                case IN_MODIFY:
                if(watch_entry->ops->in_modify)
                watch_entry->ops->in_modify(event_entry, watch_entry->path);
                break;

                case IN_MOVE_SELF:
                if(watch_entry->ops->in_move_self)
                watch_entry->ops->in_move_self(event_entry, watch_entry->path);
                break;
                
                case IN_MOVED_FROM:
                if(watch_entry->ops->in_move_from)
                watch_entry->ops->in_move_from(event_entry, watch_entry->path);
                break;

                case IN_MOVED_TO:
                if(watch_entry->ops->in_move_to)
                watch_entry->ops->in_move_to(event_entry, watch_entry->path);
                break;

                case IN_OPEN:
                if(watch_entry->ops->in_open)
                watch_entry->ops->in_open(event_entry, watch_entry->path);
                break; 

                case IN_UNMOUNT:
                if(watch_entry->ops->in_unmount)
                watch_entry->ops->in_unmount(event_entry, watch_entry->path);
                break;

                case IN_Q_OVERFLOW:
                printf ("--> sysnotify_event_handler: Events overflowed\n");
                break;
 
                default:
                break;
            }	
        }
        if(event_entry)
            free(event_entry);
        event_entry = NULL;
        watch_entry = NULL;
    }
   
    printf("---> sysnotify_event_handler end.\n");  
    return FILENOTIFY_OK;
}

int FileNotify_RegisterWatch(sFileNotifyWatchEntryList *new_entry ,int entry_count)
{
    pthread_attr_t sPthreadAttr;
    S_FileEventCount = entry_count;
    AVCT_INIT_LIST_HEAD(&S_FileEventList.list);    
    G_FileWatchEntry = (sFileNotifyWatchEntryList*) malloc(sizeof(sFileNotifyWatchEntryList) * entry_count);
    if(G_FileWatchEntry == NULL)
    {
        perror("malloc");
        return FILENOTIFY_MALLOC_FAILED;
    }
    memcpy(G_FileWatchEntry, new_entry, sizeof(sFileNotifyWatchEntryList) * entry_count);     

    if (pthread_attr_init(&sPthreadAttr) == 0)
    {
        if (pthread_attr_setdetachstate(&sPthreadAttr, PTHREAD_CREATE_DETACHED) == 0)
        {   
            pthread_create(&EventRead_thread, &sPthreadAttr, FlieNotify_EventRead, (void *)NULL);               
        }
    }
    
    if (pthread_attr_init(&sPthreadAttr) == 0)
    {        
        if (pthread_attr_setdetachstate(&sPthreadAttr, PTHREAD_CREATE_DETACHED) == 0)
        {             
            pthread_create(&EventHandler_thread, &sPthreadAttr, FlieNotify_EventHandler, (void *)NULL);   
            return FILENOTIFY_OK; 
        }
    }
    pthread_attr_destroy(&sPthreadAttr);
    return FILENOTIFY_PARAMETER_ERROR; 
}


static bool FileNotify_Open(int *fd)
{   
    *fd = inotify_init();
    if (*fd < 0)
    {
        perror("inotify_init");
        syslog( LOG_USER|LOG_ERR, "pid=%d, %s - inotify_init error(fd=%d).\n", getpid(), __FUNCTION__, *fd);
        return false;
    }
    return true;
}

bool FileNotify_Cose(int fd)
{
    if(fd >= 0)
    {
        close(fd);  
        return true;
    }
    syslog( LOG_USER|LOG_ERR, "pid=%d, %s - Invalid file descriptor.\n", getpid(), __FUNCTION__);
    return false;
}


static int FileNotify_RemoveWatch(int fd, int wd)
{
    int ret = 0;

    ret = inotify_rm_watch(fd, wd);
    if (ret < 0)
    {
        perror("inotify_rm_watch");
        syslog( LOG_USER|LOG_WARNING, "pid=%d, %s - inotify_rm_watch return = %d.\n", getpid(), __FUNCTION__, ret);
    }
    return ret;
}


static int FileNotify_ReadEevent(int fd )
{
    struct inotify_event *event = NULL;
    struct inotify_event *entry = NULL;
    unsigned char *buffer = 0;
    unsigned int uiIndex = 0;
    unsigned int event_size = 0;
    ssize_t n_read = 0;

    buffer = (unsigned char *)alloca(FILENOTIFY_MAX_EVENT_BUFSIZE);
    if(buffer == NULL)
    {
        perror("malloc");
        syslog( LOG_USER|LOG_ERR, "pid=%d, %s - malloc error.\n", getpid(), __FUNCTION__);
        return FILENOTIFY_MALLOC_FAILED;
    }
    
    n_read = read(fd, buffer, FILENOTIFY_MAX_EVENT_BUFSIZE);
    if(n_read <= 0)
    {
        free(buffer);
        syslog( LOG_USER|LOG_ERR, "pid=%d, %s - read event error(size=%d).\n", getpid(), __FUNCTION__, (int)n_read);
        return FILENOTIFY_SYSTEM_ERROR;
    }
    pthread_mutex_lock(&sysnotify_mutex);    
    while(n_read > uiIndex)
    {
        event = (struct inotify_event *)&buffer[uiIndex];
        event_size =  sizeof(struct inotify_event) + event->len;
        entry = (struct inotify_event *)malloc(event_size);

        if(entry != NULL)
        {
            memcpy(entry, event, event_size);              
            FileNotify_PushEvent(entry);            
        }            
        uiIndex += event_size;
    }    
    pthread_cond_signal(&sysnotify_cond);
    pthread_mutex_unlock(&sysnotify_mutex);
    return FILENOTIFY_OK; 
}


static int FileNotify_CheckEvent(int fd)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    return select(FD_SETSIZE, &fds, NULL, NULL, NULL);
}

static int FileNotify_PushEvent(struct inotify_event *pNoityEvent)
{
    sFileEventList *pListEntry = NULL;
    pListEntry = (sFileEventList *)malloc(sizeof(sFileEventList));
    
    if(pListEntry != NULL)
    {        
        pListEntry->e = pNoityEvent;        
        avct_list_add_tail(&(pListEntry->list), &(S_FileEventList.list));        
        return FILENOTIFY_OK; 
    }
    return FILENOTIFY_MALLOC_FAILED;
}

static int FileNotify_PopEvent(struct inotify_event **pNoityEvent)
{
    struct list_head *pos, *next;    
    avct_list_for_each_safe(pos, next, &S_FileEventList.list)
    {         
        sFileEventList *psFileEvent = NULL;        
        psFileEvent = avct_list_entry(pos, sFileEventList, list);         
        *pNoityEvent = psFileEvent->e;         
        avct_list_del(pos, &S_FileEventList.list);          
        free(psFileEvent);         
        return FILENOTIFY_OK; 
    }
    return FILENOTIFY_NULL_DATA;
}



