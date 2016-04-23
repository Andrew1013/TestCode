#define _GNU_SOURCE
#include <dlfcn.h>
#include <signal.h>
#include <execinfo.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>



/********* To Do Start************/
typedef int (*system_func)(void);
/********* To Do End************/

static system_func  _system_hook = NULL;

static int system_hook_one(system_func *fptr, const char *fname) {
	char *msg = NULL;

	assert(fname != NULL);
	if (*fptr == NULL) {
		printf("dlsym : wrapping %s\n", fname);
		*fptr = dlsym(RTLD_NEXT, fname);
		if ((*fptr == NULL) || ((msg = dlerror()) != NULL)) {
			printf("dlsym %s failed : %s\n", fname, msg);
			exit(1);
		} else {
			printf("dlsym: wrapping %s done\n", fname);
			return 0;
		}
	} else {
		return 0;
	}
	return 0;
}

void hook_funcs(void) {
	int rc = 0;
	if (_system_hook == NULL) {
		rc = system_hook_one(&_system_hook, "system"); 
		if (NULL == _system_hook || rc != 0) {
			printf("Failed to hook.\n");
			exit(EXIT_FAILURE);
		}
	}
}

/********* To Do Start************/
int system(const char *command) {
	int rc = EINVAL;
	rc = _system_hook();
	return rc;
}

/********* To Do End************/

void __my_signal_handler(int sig) {
	/********* To Do Start************/

	/********* To Do End************/
}

int register_my_signal_handler() {
	signal(10, __my_signal_handler);
	return 0;
}

void _init_sys()  __attribute__((constructor));
void _init_sys() {
	printf("*** _init().\n");
	/********* To Do Start************/

	/********* To Do End************/
	hook_funcs();
	register_my_signal_handler();
}

void  _fini_sys()  __attribute__((destructor)); 
void  _fini_sys() {
	printf("*** fini().\n");
	/********* To Do Start************/

	/********* To Do End************/
}

