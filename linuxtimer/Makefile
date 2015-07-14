CC = gcc
CFLAGS += -Wall -fPIC -Werror -Wshadow -O2 -Wformat -Wno-format-extra-args -Wformat-security -Wformat-nonliteral -Wformat=2 -fstack-protector -D_FORTIFY_SOURCE=2 
INC = -I.
LIBS = -lrt -lpthread

OBJECTS = threadtimer.o timerlib.o
#${OBJECTS}: ${SOURCES}

all : Timer

Timer : ${OBJECTS} 
	${CC} ${CFLAGS} -o $@ ${OBJECTS} ${LIBS}

%.o: %.c
	${CC} ${CFLAGS} -c $< -o $@

%.o: %.cpp
	${CC} ${CFLAGS} -c $< -o $@
	
clean:
	rm -f *.o 
	rm -f Timer
