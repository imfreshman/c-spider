ROOT_DIR=$(shell pwd)

ver=debug

SOURCES=${wildcard *.c}
OBJS=${patsubst %.c,%.o,$(SOURCES)}
DYLIBNAMES=${patsubst %.c,%.so, $(SOURCES)}

ifeq ($(ver), debug)
	COMFLAGS = -fpic -c -g -O0 -Wall -DUX_DEBUG 
else
	COMFLAGS = -fpic -c -O3 -Wall
endif

CFLAGS = -I $(ROOT_DIR)/../src/include

CC=gcc

#OBJS_DIR=$(ROOT_DIR)/../src/debug/obj

MODULES_DIR=$(ROOT_DIR)/../src/debug/bin/modules

all: $(DYLIBNAMES)

%.so:%.o
	$(CC) -shared  $< -o $(MODULES_DIR)/$@ 

.c.o:
	$(CC) $(COMFLAGS) ./$< -o $@ $(CFLAGS)

clean:
	rm -rf $(OBJS)
	rm -rf $(MODULES_DIR)/$(DYLIBNAMES)