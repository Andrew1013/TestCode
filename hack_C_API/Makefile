

TARGET      = mylibrary


CPPFLAGS        = 
LDFLAGS         = 
CFLAGS = -Wall -Werror -Wshadow -O2 -Wformat -Wno-format-extra-args -Wformat-security -Wformat-nonliteral -Wformat=2 -fstack-protector -D_FORTIFY_SOURCE=2 -fPIC

LIB_SOURCES   = mylibrary.c 
LIB_OBJECTS  := $(patsubst %.c,%.o,$(LIB_SOURCES))
LIB_REAL_NAME      := lib$(TARGET).so.$(LIB_VER_IFACE).$(LIB_VER_MINOR).$(LIB_VER_RELEASE)
LIB_SO_NAME        := lib$(TARGET).so.$(LIB_VER_IFACE)
LIB_LINK_NAME      := lib$(TARGET).so
LIB_LIBRARIES = -Wl,--allow-shlib-undefined -lpthread -ldl

#
# unittest definitions
#
UT_EXE       = generate_hacked_api
UT_SOURCES   = generate_hacked_api.c 
UT_OBJS	    := $(UT_SOURCES:.c=.o)
UT_LIBRARIES = -Wl,--allow-shlib-undefined -lpthread
UT_LIBPATH   = -L. 


#
# definitions
#
CROSS       = $(_EX_CROSS_COMPILE)
CC      	= $(CROSS)gcc
LD      	= $(CROSS)ld
AR      	= $(CROSS)ar
RANLIB  	= $(CROSS)ranlib
STRIP		= $(CROSS)strip --strip-unneeded

#
# make rule
#
.PHONY:	all 
all: build install

lib: $(LIB_REAL_NAME)
build:  $(UT_EXE)

$(LIB_REAL_NAME): $(LIB_OBJECTS)
	$(CC) -shared $^ $(LDFLAGS) $(LIB_CFLAG) $(LIBPATH) $(CFLAGS) $(LIB_LIBRARIES) -Wl,-soname,$(LIB_SO_NAME) -o $@
	ln -sf    $(LIB_REAL_NAME)  $(LIB_SO_NAME)
	ln -sf    $(LIB_REAL_NAME)  $(LIB_LINK_NAME)

$(UT_EXE):$(UT_OBJS)
	$(CC) $(UT_OBJS) $(CFLAGS) $(CPPFLAGS) $(UT_LIBPATH) $(UT_LIBRARIES) $(LDFLAGS)  -o $@ 


#
# install to rootfs
#
.PHONY:	install
install:


#
# uninstall to rootfs
#
.PHONY:	uninstall
uninstall:

#
# clean
#
.PHONY:	clean
clean:
	rm -f *.o
	rm -f *.so*
	rm -f $(UT_EXE)


