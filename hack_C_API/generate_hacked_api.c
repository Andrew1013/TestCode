#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define API_NAME_LENGTH 100

typedef struct {
	char name[API_NAME_LENGTH];
} apistruct;

apistruct *apis = NULL;
int signum = 0;
FILE *fp = NULL;
int api_count = 0;

void usage()
{
	printf("Usage:\n");
	printf("generate_hacked_api [OPTIONS] ...\n");
	printf("-a <API1>,<API2>,<API3>, ....\n");
	printf("-s <SIGNUM> : register signal handler\n");
	printf("-f <file name> : output file name, default file name is mylibrary.c\n");
	printf("-h : this document\n");
	printf("Note: You can specify multiple API names for -a argument, but you only can specify one signal handler\n");
	exit(1);	
}

int getAPICount(char *opt)
{
	int count = 0;
	char *ptr = opt;
	
	while (*ptr != 0)
	{
		if (*ptr == ',')
			count++;
		ptr++;
	}
	
	/* if the last character is not ',', to increase count with 1 */
	if (*(ptr - 1) != ',')
		count++;
	return count;
}

int parse_api_name(char *opt)
{
	int i = 0;
	char *ptr, *next;
	const char *delim = ",";
	char local_buf[strlen(opt)+1];
	
	snprintf(local_buf, sizeof(local_buf), "%s", opt);

	api_count = getAPICount(opt);
	apis = calloc(sizeof(apistruct) * api_count, 1);

	ptr = local_buf;
	while (ptr)
	{
		next = strsep(&ptr, delim);
		snprintf(apis[i].name, sizeof(apis[i].name), "%s", next);
		i++;
	}

	if (i != api_count)
	{
		printf("API count is not match, error ...\n");
		exit(1);
	}

	return 0;
}

int parse_sig_name(char *opt)
{
	signum = atoi(opt);
	return 0;
}

int generate_code_header()
{
	int i;
	fprintf(fp, "#define _GNU_SOURCE\n#include <dlfcn.h>\n#include <signal.h>\n");
	fprintf(fp, "#include <execinfo.h>\n#include <errno.h>\n#include <stdlib.h>\n");
	fprintf(fp, "#include <stdio.h>\n#include <unistd.h>\n#include <pthread.h>\n#include <assert.h>\n\n\n");

	fprintf(fp, "/********* To Do Start************/\n");
	for (i = 0;i < api_count; i++)
	{
		fprintf(fp, "typedef int (*%s_func)(void);\n", apis[i].name);
	}
	fprintf(fp, "/********* To Do End************/\n\n");
	
	for (i = 0;i < api_count; i++)
	{
		fprintf(fp, "static %s_func  _%s_hook = NULL;\n", apis[i].name, apis[i].name);
	}
	fprintf(fp, "\n");

	return 0;
}

int generate_code_tail()
{
	fprintf(fp, "void _init()  __attribute__((constructor));\n");
	fprintf(fp, "void _init() {\n");
	fprintf(fp, "\tprintf(\"*** _init().\\n\");\n");
	fprintf(fp, "\t/********* To Do Start************/\n");
	fprintf(fp, "\n");
	fprintf(fp, "\t/********* To Do End************/\n");
	fprintf(fp, "\thook_funcs();\n");
	if (signum)
		fprintf(fp, "\tregister_my_signal_handler();\n");
	fprintf(fp, "}\n");

	fprintf(fp, "\n");
	fprintf(fp, "void  _fini()  __attribute__((destructor)); \n");
	fprintf(fp, "void  _fini() {\n");
	fprintf(fp, "\tprintf(\"*** fini().\\n\");\n");
	fprintf(fp, "\t/********* To Do Start************/\n");
	fprintf(fp, "\n");
	fprintf(fp, "\t/********* To Do End************/\n");
	fprintf(fp, "}\n");
	fprintf(fp, "\n");
	
	return 0;	
}

int generate_signal_handler()
{
	fprintf(fp, "\n");
	fprintf(fp, "void __my_signal_handler(int sig) {\n");
	fprintf(fp, "\t/********* To Do Start************/\n");
	fprintf(fp, "\n");
	fprintf(fp, "\t/********* To Do End************/\n");
	fprintf(fp, "}\n");
	fprintf(fp, "\n");

	fprintf(fp, "int register_my_signal_handler() {\n");
	fprintf(fp, "\tsignal(%d, __my_signal_handler);\n", signum);
	fprintf(fp, "\treturn 0;\n");
	fprintf(fp, "}\n\n");

	return 0;
}

int generate_hook_func(char *fname)
{
	fprintf(fp, "\tif (_%s_hook == NULL) {\n", fname);
	fprintf(fp, "\t\trc = %s_hook_one(&_%s_hook, \"%s\"); \n", fname, fname, fname);
	fprintf(fp, "\t\tif (NULL == _%s_hook || rc != 0) {\n", fname);
	fprintf(fp, "\t\t\tprintf(\"Failed to hook.\\n\");\n");
	fprintf(fp, "\t\t\texit(EXIT_FAILURE);\n");
	fprintf(fp, "\t\t}\n");
	fprintf(fp, "\t}\n");
	return 0;
}

int generate_hook_code_body(char *fname)
{
	fprintf(fp, "static int %s_hook_one(%s_func *fptr, const char *fname) {\n", fname, fname);
	fprintf(fp, "\tchar *msg = NULL;\n");
	fprintf(fp, "\n");
	fprintf(fp, "\tassert(fname != NULL);\n");
	fprintf(fp, "\tif (*fptr == NULL) {\n");
	fprintf(fp, "\t\tprintf(\"dlsym : wrapping %%s\\n\", fname);\n");
	fprintf(fp, "\t\t*fptr = dlsym(RTLD_NEXT, fname);\n");
	fprintf(fp, "\t\tif ((*fptr == NULL) || ((msg = dlerror()) != NULL)) {\n");
	fprintf(fp, "\t\t\tprintf(\"dlsym %%s failed : %%s\\n\", fname, msg);\n");
	fprintf(fp, "\t\t\texit(1);\n");
	fprintf(fp, "\t\t} else {\n");
	fprintf(fp, "\t\t\tprintf(\"dlsym: wrapping %%s done\\n\", fname);\n");
	fprintf(fp, "\t\t\treturn 0;\n");
	fprintf(fp, "\t\t}\n");
	fprintf(fp, "\t} else {\n");
	fprintf(fp, "\t\treturn 0;\n");
	fprintf(fp, "\t}\n");
	fprintf(fp, "\treturn 0;\n");
	fprintf(fp, "}\n\n");

	return 0;
}

int generate_hack_func(char *fname)
{
	fprintf(fp, "int %s(void) {\n", fname);
	fprintf(fp, "\tint rc = EINVAL;\n");
	fprintf(fp, "\trc = _%s_hook();\n", fname);
	fprintf(fp, "\treturn rc;\n");
	fprintf(fp, "}\n\n");

	return 0;
}

int generate_code_body()
{
	int i;
		for (i = 0;i < api_count; i++)
	{
		generate_hook_code_body(apis[i].name);
	}
	
	fprintf(fp, "void hook_funcs(void) {\n");
	fprintf(fp, "\tint rc = 0;\n");
	for (i = 0;i < api_count; i++)
	{
		generate_hook_func(apis[i].name);
	}
	fprintf(fp, "}\n\n");

	fprintf(fp, "/********* To Do Start************/\n");
	for (i = 0;i < api_count; i++)
	{
		generate_hack_func(apis[i].name);
	}
	fprintf(fp, "/********* To Do End************/\n");
	
	return 0;
}

int main(int argc, char *argv[])
{
	int opt;
	char *filename = "mylibrary.c";

	if (argc == 1)
	{
		usage();
	}
	else if (argc == 2)
	{
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
		{
			usage();
		}
	}

	while ((opt = getopt(argc, argv, "a:s:f:h")) != -1)
	{
		switch (opt)
		{
			case 'h':
				usage();
				return 0;
			case 'a':
				parse_api_name(optarg);
				break;
			case 's':
				parse_sig_name(optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			default:
				usage();
		}
	}
	fp = fopen(filename, "w+");
	
	if (fp == NULL)
	{
		printf("can't open file %s\n", filename);
		exit(1);
	}
	
	generate_code_header();
	generate_code_body();
	if (signum)
		generate_signal_handler();
	generate_code_tail();

	fclose(fp);
	return 0;	
}
