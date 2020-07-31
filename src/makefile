#Makefile

ROOT_DIR=$(shell pwd)

ver=debug

ifeq ($(ver), debug)
	COMFLAGS = -fpic -c -g -O0 -Wall -DUX_DEBUG 
else
	COMFLAGS = -fpic -c -O3 -Wall
endif

CFLAGS = -I $(ROOT_DIR)/include

LDFLAGS += -pthread -ldl -rdynamic -levent

CC=gcc

SUBDIRS=main config cstring bloomfilter crc32 dso epoll hash threads socket url
BIN=spider

OBJS_DIR=debug/obj

BIN_DIR=debug/bin

export	CC BIN OBJS_DIR BIN_DIR	COMFLAGS CFLAGS  LDFLAGS ROOT_DIR 

all:$(SUBDIRS) DEBUG

$(SUBDIRS):ECHO
	make -C $@

DEBUG:ECHO
	make -C debug

ECHO:
	@echo $(SUBDIRS)

clean:
	@rm $(OBJS_DIR)/*.o
	@rm -rf $(BIN_DIR)/$(BIN)


