CC      = gcc
CPP     = g++
AR = ar
RM      = rm -f
#scl enable devtoolset-7 bash
#Define the resource compiler.
RC = windres

## debug flag  
DBG_ENABLE := 1

OS = $(shell uname)

## source file path  
SRC_PATH  := ./ ../memory/ ../common/ ../http/ ../cjson/
SRC_PATH_EXT := 

## target file name  
TARGET     := ../release/ocean_ty
#TARGET     := ./ocean_ty

## get all source files  
SRCS := $(foreach spath, $(SRC_PATH), $(wildcard $(spath)*.c) )	$(foreach spath, $(SRC_PATH), $(wildcard $(spath)*.cpp) )

## all .o based on all .c/.cpp
OBJS = $(SRCS:.c=.o)
OBJS := $(OBJS:.cpp=.o) 

## macro define
DEFS := __LINUX__  OS_LINUX _FILE_OFFSET_BITS=64 $(if $(findstring D,$(ENCYPT)), _USE_ENCYPTION_DONGLE, ) $(if $(findstring t,$(ENCYPT)), _DONGLE_CHECK_TIME, )

#if freeimage is static-linked use this !
#DEFS += FREEIMAGE_LIB

## need libs, add at here  rdkafka++
LIBS := rt rmmt_sdk2 curl tbb uuid opencv_core opencv_imgproc opencv_highgui apr-1 activemq-cpp rdkafka  hiredis event sqlite3

## used headers  file path  
INCLUDE_PATH := ../include ../ab_pkts ../http ../memory ../common /usr/local/cuda/include /usr/local/include ./ ../include/activemq  /usr/include/apr-1 ../include/librdkafka/src ../cjson ../include/libevent/include 

#$(warning $(INCLUDE_PATH))

## used include librarys file path  
LIBRARY_PATH := ./obj ../lib /usr/local/lib /usr/local/cuda/lib64
 
## debug for debug info, when use gdb to debug  
ifeq (1, ${DBG_ENABLE})   
CFLAGS += -O0 -D_DEBUG -g -DDEBUG=1 
else
CFLAGS += -O3 -DNDEBUG
endif

#for ENCYPT flags
ifeq ($(shell uname), Linux)
LIBRARY_PATH += ./
LIBS +=  $(if $(findstring D,$(ENCYPT)), RockeyARM, )
CFLAGS += -fPIC
else
LIBRARY_PATH += ./lib
#DEFS += _WIN32
LIBS +=  ws2_32 $(if $(findstring D,$(ENCYPT)), Dongle_d, )
endif

ifeq ($(OS), Linux)
LIBS += dl
CFLAGS += -fPIC
TARGET_EXT := 
LIBRARY_PATH += 
LDFLAGS += -Wl,--rpath=./obj
endif

CFLAGS += -march=corei7 -msse4.2 -pipe $(foreach m, $(DEFS), -D$(m)) 
  
## get all include path  
CFLAGS  += $(foreach dir, $(INCLUDE_PATH), -I$(dir))  -g -rdynamic   

CXXFLAGS += $(CFLAGS) -std=c++11

## get all library path  
LDFLAGS += -lpthread $(foreach lib, $(LIBRARY_PATH), -L$(lib))
  
## get all librarys  
LDFLAGS += $(foreach lib, $(LIBS), -l$(lib))

RCFLAGS ?= -DNDEBUG


default: all

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CPP) $(CXXFLAGS) -c $< -o $@
	
all: $(OBJS) $(RESOURCE)
	$(CPP) $(CXXFLAGS) -o $(TARGET)$(TARGET_EXT) $(OBJS) $(RESOURCE) $(LDFLAGS)

clean:  
	$(RM) $(OBJS) $(TARGET) $(RESOURCE)
