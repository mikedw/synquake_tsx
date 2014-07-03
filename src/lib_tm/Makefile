####################################################################################################
##
## CONFIGURATION
##
####################################################################################################

## Compilation parameters
CC = gcc
CXX = g++

ifeq ($(DEBUG), )
	DEBUG = -Wall
endif

#LibTM Options
OPTIONS = 

CXXFLAGS := $(DEBUG) -O3 -mrtm $(VERSION) $(OPTIONS)
LDFLAGS = -pthread -lstdc++


## Source and binary files
BUILD_DIR   =   build

C_SRCS    =   $(wildcard *.c)
C_OBJS    =   $(patsubst %.c,$(BUILD_DIR)/%.o,$(C_SRCS)) 
CC_SRCS   =   $(wildcard *.cc)
CC_OBJS   =   $(patsubst %.cc,$(BUILD_DIR)/%.o,$(CC_SRCS)) 


all: lib_tm.a Makefile

init:
					mkdir -p $(BUILD_DIR)

lib_tm.a: init $(C_OBJS) $(CC_OBJS)
					rm -f $@
					ar -qc $@ $(C_OBJS) $(CC_OBJS)

# For C source files
$(BUILD_DIR)/%.o: %.c Makefile
	${CXX} -c ${CXXFLAGS} -o $@ $<

# For CC source files
$(BUILD_DIR)/%.o: %.cc Makefile
	${CXX} -c ${CXXFLAGS} -o $@ $<

default: all 

clean:
		-rm -f -R $(BUILD_DIR)
		-rm -f lib_tm.a

clean_all:	clean

.PHONY: all clean

