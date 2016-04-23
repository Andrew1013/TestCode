#------------------------------------------------------------------------------
# set active module
#------------------------------------------------------------------------------
ACTIVE_MODULE += example
ACTIVE_MODULE += lib

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

