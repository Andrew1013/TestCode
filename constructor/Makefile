CC = gcc
CFLAGS = -g 
INC = -I.
LIBS = 

OBJECTS = constructor.o

#${OBJECTS}: ${SOURCES}

all : constructor

constructor : ${OBJECTS} 
	${CC} ${CFLAGS} -o $@ ${OBJECTS} ${LIBS}
	
%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.cpp
	${CC} ${CFLAGS} -c $< -o $@
	
clean:
	rm -f *.o 
	rm -f constructor
