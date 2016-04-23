CC = gcc
CFLAGS = -g 
INC = -I.
LIBS = 

OBJECTS = test_list.o

#${OBJECTS}: ${SOURCES}

all : build

build: test1

test1 : ${OBJECTS} 
	${CC} ${CFLAGS} -o $@ ${OBJECTS} ${LIBS}
	
%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.cpp
	${CC} ${CFLAGS} -c $< -o $@
	
clean:
	rm -f *.o 
	rm -f test1
