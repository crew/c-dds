CC=gcc
OUT=run
CFLAGS=-c
BUILDDIR=build
SRCS=main.c dict.c readcfg.c
OBJS=$(SRCS:.c=.o)
all : $(OBJS)
	$(CC) $(OBJS) -o ../$(BUILDDIR)/$(OUT)
	make clean

$(OBJS) : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

clean : 
	rm -rf $(OBJS)

