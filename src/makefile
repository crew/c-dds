CC=gcc
OUT=run
CFLAGS=-c
BUILDDIR=build
SRCS=main.c dict.c readcfg.c parseJSON.c cJSON.c ddsmsgqueue.c
ENDARGS=-lrt -lm
OBJS=$(SRCS:.c=.o)
all : $(OBJS)
	$(CC) $(OBJS) -o ../$(BUILDDIR)/$(OUT) $(ENDARGS)
	make clean

$(OBJS) : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

clean : 
	rm -rf $(OBJS)

