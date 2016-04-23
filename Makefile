#------------------------------------------------------------------------------
# set active module
#------------------------------------------------------------------------------
ACTIVE_MODULE += lockdep
ACTIVE_MODULE += semrwlock
ACTIVE_MODULE += list
ACTIVE_MODULE += linuxtimer
ACTIVE_MODULE += filenotifylib
ACTIVE_MODULE += constructor
ACTIVE_MODULE += hack_C_API
#------------------------------------------------------------------------------
# make rules
#------------------------------------------------------------------------------
.PHONY: all build clean 

all: build install

build:
	@for act in $(ACTIVE_MODULE);     \
	do                                \
		(cd $$act && $(MAKE) $@); \
		if [ $$? != 0 ]; then     \
			exit 1;           \
		fi                        \
	done

clean:
	@for act in $(ACTIVE_MODULE);     \
	do                                \
		(cd $$act && $(MAKE) $@); \
	done
	
cleanall:
	@for act in $(ACTIVE_MODULE);     \
	do                                \
		(cd $$act && $(MAKE) $@); \
	done

install:
	@for act in $(ACTIVE_MODULE);     \
	do                                \
		(cd $$act && $(MAKE) $@); \
	done

uninstall:
	@for act in $(ACTIVE_MODULE);     \
	do                                \
		(cd $$act && $(MAKE) $@); \
	done

