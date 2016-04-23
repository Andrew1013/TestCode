# Code generatation for hacking C API
LD_PRELOAD is general way to load an additional, user-specified, ELF shared library before executing a binray.
It can be used to override functions in other shared objects.

In this repository, it provides a code generatation code which can generate a code template for hacking a specified API.
And some examples for describing how to use it.

FILE lists are:

1. generate_hacked_api.c : code generatation function (make)
2. mylibrary.c : example for hacking system API (make lib)
  

# How to use code generatation
Usage:<br>
generate_hacked_api [OPTIONS] ... <br>
-a API1,API2,API3, ....<br>
-s <SIGNUM> : register signal handler<br>
-f <file name> : output file name, default file name is mylibrary.c<br>
-h : this document<br>
Note: You can specify multiple API names for -a argument, but you only can specify one signal handler<br>

Example:<br>
./generate_hacked_api -a system -s 10 -f hacked_system.c

In this example, a C file hacked_system.c which can be used to hack system API will be generated. At the same time, a signal handler which registers SIGUSR1 (10) is also generated.

# How to use a hacked library
LD_PRELOAD=./hacked_system.so ./system_test<br>
process ./system_test is calling system<br>

