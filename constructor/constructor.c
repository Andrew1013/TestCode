#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>

static bool G_bQuit = false;

void __attribute__((constructor)) init_function(void);
void  __attribute__((destructor)) fnit_function(void);


void init_function()
{
	printf("%s\n", __FUNCTION__);
}

void fnit_function()
{
	printf("%s\n", __FUNCTION__);
}

void got_signal(int a)
{
	G_bQuit = true;
}

int main(int argc, char **argv)
{
	struct sigaction sa;
	memset( &sa, 0, sizeof(sa) );
	sa.sa_handler = got_signal;
	sigfillset(&sa.sa_mask);
	sigaction(SIGINT,&sa,NULL);
	sigaction(SIGTERM ,&sa,NULL);
	printf("%s .... start\n\n", __FUNCTION__);
	printf("Type Ctrl-c or killall constructor to test\n\n" );
    while(true)
    {
        sleep(20);
		if(G_bQuit)
			break;
    }
    printf("\n%s .... end\n", __FUNCTION__);
    return 0;
}


