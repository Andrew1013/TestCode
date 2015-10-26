#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <syslog.h>
#include <libgen.h>

void __attribute__((constructor)) init_fd_check_task(void);
void  __attribute__((destructor)) fnit_fd_check_task(void);
static void GetFDInfo();
static void PrintFDInfo( int fd );
static char *pid_to_name(pid_t pid, char* buf, int buffsize);
static FILE *fpchk = NULL;  

#define FD_CHECK_PATH  "/tmp/fdcheck/"
#define MAX_STR_LEN 512


void *FdCheck_Thread(void *arg)
{
    char buffer[128];
    while(1)
    {
        time_t t1 = time(NULL);
        snprintf(buffer, sizeof(buffer), "%s", ctime(&t1));
        fwrite(buffer, 1, strlen(buffer), fpchk);
        GetFDInfo();
        sleep(60);
    }    
}

void init_fd_check_task()
{
    
    pthread_attr_t sPthreadAttr;      
    pthread_t FdCheck_thread = 0;
    char tmpBuffer[64];
    char tmpBuffer1[64];
    
    if(pid_to_name(getpid(), tmpBuffer, sizeof(tmpBuffer)) == NULL)
    {
        syslog( LOG_USER | LOG_WARNING ,"%s pid_to_name failed\n", __FUNCTION__);
        return;
    }
    snprintf(tmpBuffer1, sizeof(tmpBuffer1), "%s%s", FD_CHECK_PATH, basename(tmpBuffer));

    mkdir(FD_CHECK_PATH, 666);
    fpchk  = fopen(tmpBuffer1, "a+");      
    if(fpchk == NULL)
    {
        syslog( LOG_USER | LOG_WARNING ,"%s open failed\n", __FUNCTION__, tmpBuffer1);
        return;
    }
          
    if (pthread_attr_init(&sPthreadAttr) == 0)
    {
        if (pthread_attr_setdetachstate(&sPthreadAttr, PTHREAD_CREATE_DETACHED) == 0)
        {   
            pthread_create(&FdCheck_thread, &sPthreadAttr, FdCheck_Thread, (void *)NULL);               
        }
    }
    
}

static char *pid_to_name(pid_t pid, char* buf, int buffsize)
{    
    FILE *fp;    
    char cmdline[256];    
    bool retBuf = false;    
    snprintf(cmdline, sizeof(cmdline), "/proc/%d/cmdline", (int)pid);    
    fp  = fopen(cmdline, "r");  
    if(fp)    
    {        
        if ( 0 < fread(buf, sizeof(char), buffsize, fp) )        
        {            
            retBuf = true;        
        }        
        fclose(fp);    
    }    
    if (retBuf == true)    
    {        
        return buf;    
    }    
    else    
    {        
        return NULL;    
    }
}


void fnit_fd_check_function()
{
    if(fpchk)
	    fclose(fpchk);
}

static void PrintFDInfo( int fd )
{
	char buf[MAX_STR_LEN];
    char buf1[64];    
	
	int fd_flags = fcntl( fd, F_GETFD ); 
	if ( fd_flags == -1 ) 
		return;
	
	int fl_flags = fcntl( fd, F_GETFL ); 
	if ( fl_flags == -1 ) 
		return;
	
	char path[256];
	snprintf( path, sizeof(path),"/proc/self/fd/%d", fd );
	
	memset(buf1, 0, sizeof(buf1));
    memset(buf, 0, sizeof(buf)); 
    
	ssize_t s = readlink( path, &buf1[0], sizeof(buf1) );
	if ( s == -1 )
	{        
		printf("(%s): not available\n", path);
		return;
	}    
    snprintf(buf, sizeof(buf1), "%d (%s)  ", fd, buf1);
    
#define BUFFER_LEN (MAX_STR_LEN - strlen(buf) -1)

	if ( fd_flags & FD_CLOEXEC )  strncat(buf, "cloexec ", BUFFER_LEN); 
	
	// file status
	if ( fl_flags & O_APPEND   )  strncat(buf, "append ", BUFFER_LEN);
	if ( fl_flags & O_NONBLOCK )  strncat(buf, "nonblock ", BUFFER_LEN);
	
	// acc mode
	if ( fl_flags & O_RDONLY   )  strncat(buf, "read-only ", BUFFER_LEN); 
	if ( fl_flags & O_RDWR     )  strncat(buf, "read-write ", BUFFER_LEN); 
	if ( fl_flags & O_WRONLY   )  strncat(buf, "write-only ", BUFFER_LEN); 
	
	if ( fl_flags & O_DSYNC    )  strncat(buf, "dsync ", BUFFER_LEN); 
	if ( fl_flags & O_RSYNC    )  strncat(buf, "rsync ", BUFFER_LEN); 
	if ( fl_flags & O_SYNC     )  strncat(buf, "rsync ", BUFFER_LEN); 
	
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_whence = 0;
	fl.l_start = 0;
	fl.l_len = 0;
	fcntl( fd, F_GETLK, &fl );
	if ( fl.l_type != F_UNLCK )
	{
		if ( fl.l_type == F_WRLCK )
            strncat(buf, "write-locked ", BUFFER_LEN); 			         
		else
            strncat(buf, "read-locked ", BUFFER_LEN); 			

        snprintf(buf1, sizeof(buf1), "(pid:%d) ", fl.l_pid);
        strncat(buf, buf1, BUFFER_LEN); 		       
	}
    strncat(buf, "\n", BUFFER_LEN);     
    fwrite(buf, 1, MAX_STR_LEN, fpchk);
    
#undef BUFFER_LEN    
}

static void GetFDInfo()
{
	int i = 0;	
	int numHandles = getdtablesize(); 	
	for (  i = 0; i < numHandles; i++ )
	{
		int fd_flags = fcntl( i, F_GETFD );		
		if ( fd_flags == -1 ) 
			continue;
		
		PrintFDInfo( i );
	}
} 


