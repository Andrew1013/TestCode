# Simple Makefile to build lcdut executable

# Use native gcc by default.
# This is overridden by OE for cross compiling.

CC                  := gcc
CFLAGS              := -Wall -fPIC -O -g
LDFLAGS							:= -ldl -lpthread -rdynamic
DESTDIR             := $(PWD)/output
bindir              := /usr/bin

TARGET              = lockdep

LIB_SRCS            = lockdep_lib.c
LIB_SOURCES_OBJS    := $(patsubst %.c,%.o,$(LIB_SRCS))





LIB_VER_IFACE      = 1
LIB_VER_MINOR      = 0
LIB_VER_RELEASE    = 0
LIB_REAL_NAME      := lib$(TARGET).so.$(LIB_VER_IFACE).$(LIB_VER_MINOR).$(LIB_VER_RELEASE)
LIB_SO_NAME        := lib$(TARGET).so.$(LIB_VER_IFACE)
LIB_LINK_NAME      := lib$(TARGET).so



default: $(LIB_REAL_NAME) 
all: build

build: $(LIB_REAL_NAME)

%.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $<

$(LIB_REAL_NAME): $(LIB_SOURCES_OBJS)
	$(CC) -shared  $(CFLAGS) $(LDFLAGS) $(LIB_SOURCES_OBJS) -Wl,-soname,$(LIB_SO_NAME)  -o $@
	ln -fs $(LIB_REAL_NAME) $(LIB_SO_NAME)
	ln -fs $(LIB_REAL_NAME) $(LIB_LINK_NAME)
    

.PHONY: clean
clean:
	@echo "clean component"
	rm  -rf  $(LIB_SOURCES_OBJS) $(LIB_REAL_NAME) $(LIB_SO_NAME) $(LIB_LINK_NAME) 

install: $(LIB_REAL_NAME) 


