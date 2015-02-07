BUILDDIR=../build/plugins
SRC=dds_plugins.c
INCLUDES= -I..
OBJS=$(SRC:.c=.o)
CFLAGS=-c $(INCLUDES)
CC=gcc
all : $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BUILDDIR)

