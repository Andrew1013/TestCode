#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "filenotify.h"


void displayInotifyEvent(struct inotify_event *i)
{
    printf("    wd =%2d; ", i->wd);
    if (i->cookie > 0)
        printf("cookie =%4d; ", i->cookie);

    printf("mask = ");
    if (i->mask & IN_ACCESS)        printf("IN_ACCESS ");
    if (i->mask & IN_ATTRIB)        printf("IN_ATTRIB ");
    if (i->mask & IN_CLOSE_NOWRITE) printf("IN_CLOSE_NOWRITE ");
    if (i->mask & IN_CLOSE_WRITE)   printf("IN_CLOSE_WRITE ");
    if (i->mask & IN_CREATE)        printf("IN_CREATE ");
    if (i->mask & IN_DELETE)        printf("IN_DELETE ");
    if (i->mask & IN_DELETE_SELF)   printf("IN_DELETE_SELF ");
    if (i->mask & IN_IGNORED)       printf("IN_IGNORED ");
    if (i->mask & IN_ISDIR)         printf("IN_ISDIR ");
    if (i->mask & IN_MODIFY)        printf("IN_MODIFY ");
    if (i->mask & IN_MOVE_SELF)     printf("IN_MOVE_SELF ");
    if (i->mask & IN_MOVED_FROM)    printf("IN_MOVED_FROM ");
    if (i->mask & IN_MOVED_TO)      printf("IN_MOVED_TO ");
    if (i->mask & IN_OPEN)          printf("IN_OPEN ");
    if (i->mask & IN_Q_OVERFLOW)    printf("IN_Q_OVERFLOW ");
    if (i->mask & IN_UNMOUNT)       printf("IN_UNMOUNT ");
    printf("\n");

    if (i->len > 0)
        printf("        name = %s\n", i->name);
}


void handler1(struct inotify_event *e, char* path);
void handler2(struct inotify_event *e, char* path);


struct sFileNotify_watch_ops notify_ops1 =
{
    .in_access = handler1,
    .in_attribute = handler1,    
    .in_close_nowr = handler1,
    .in_close_wr = handler1,
    .in_create = handler1,
    .in_del = handler1,
    .in_del_self = handler1,
    .in_ignored  = handler1,
    .in_isdir = handler1,
    .in_modify = handler1,
    .in_move_self = handler1,    
    .in_move_from = handler1,
    .in_move_to = handler1,
    .in_open = handler1,
    .in_unmount = handler1
};

struct sFileNotify_watch_ops notify_ops2 =
{
    .in_access = handler2,
    .in_attribute = handler2,    
    .in_close_nowr = handler2,
    .in_close_wr = handler2,
    .in_create = handler2,
    .in_del = handler2,
    .in_del_self = handler2,
    .in_ignored  = handler2,
    .in_isdir = handler2,
    .in_modify = handler2,
    .in_move_self = handler2,    
    .in_move_from = handler2,
    .in_move_to = handler2,
    .in_open = handler2,
    .in_unmount = handler2
};


int main(int argc, char **argv)
{
    sFileNotifyWatchEntryList entry[2];    
    memset(&entry, 0, sizeof(sFileNotifyWatchEntryList) * 2);
    int ret = 0;
    ret = system("/bin/touch  /tmp/test1");
    if(WEXITSTATUS(ret) == 0)
        printf("The  /tmp/test1 has been created, ret:%d\n", ret);
    ret = system("/bin/touch  /tmp/test2");
    if(WEXITSTATUS(ret) == 0)
        printf("The  /tmp/test2 has been created, ret:%d\n", ret);

    snprintf(entry[0].path, FILENOTIFY_MAX_PATH, "/tmp/test1");    
    entry[0].ops = &notify_ops1;
    entry[0].mask = IN_ALL_EVENTS;

    snprintf(entry[1].path, FILENOTIFY_MAX_PATH, "/tmp/test2");    
    entry[1].ops = &notify_ops2;
    entry[1].mask = IN_ALL_EVENTS;

    
    if(FileNotify_RegisterWatch(entry, 2) == FILENOTIFY_OK)
        printf("--> FileNotify_RegisterWatch is successfully. pid:%d \n", getpid());

    sleep(60);
    FileNotify_StopWatch("/tmp/test2");
    printf("--> Stop Watch /tmp/test2.\n");


    sleep(90);
    FileNotify_RemoveRegisterWatch();
    printf("--> FileNotify_RemoveRegisterWatch \n");


    while(1)
        sleep(10);

    return 0;
}


void handler1(struct inotify_event *e, char* path)
{    
    printf("path: %s\n", path);
    displayInotifyEvent(e);    
}

void handler2(struct inotify_event *e, char* path)
{
    printf("path: %s\n", path);
    displayInotifyEvent(e);
}
