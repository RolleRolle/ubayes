# Makefile for C libraries
# by Mark A. Post, November 2010

# Sources, Binaries, Libraries
SRCS	= node.c graph.c logic.c
OBJS	= $(SRCS:.c=.o)
LIBS	= libubayes.a
BIN	= testnode testsensor rangeavoid testlogic hybridlogic logictree logicpgm
LDLIBS	= -lubayes -lumath -lm

# Compiler, Linker Defines
CC	= gcc
AR	= ar
LDFLAGS	= -L. -L../umath
ifdef DEBUG
	CFLAGS	= -Wall -g -I../umath -DDEBUG 
else
	CFLAGS	= -Wall -O2 -I../umath
endif

# Declarations
.PHONY: all clean

# Compile and Assemble C Source Files into Object Files
all: $(LIBS) $(BIN)

# Clean Up Objects, Binaries out of source directory
clean:
	rm -f $(OBJS) $(LIBS) $(BIN)

libubayes.a: $(OBJS)
	$(AR) rcs $@ $?

gentables: gentables.o
	$(CC) -Wall $? -o $@ -lm

