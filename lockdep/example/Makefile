CC=gcc
CFLAGS=   -Wall -fPIC -O -g 
OBJECTS= hello_mutex.o 
LIBS = -lpthread -ldl  -rdynamic


# --- targets
all:    build

build: lockdep_test


lockdep_test:$(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LIBS) $(LDFLAGS)  -o $@        
       
       
install:


# --- remove binary and executable files
clean:
	rm -f $(OBJECTS) lockdep_test
