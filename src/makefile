CC=gcc
OUT=run
CFLAGS=-c `pkg-config --cflags --libs gtk+-3.0 webkitgtk-3.0`
BUILDDIR=build
SRCS=main.c dict.c readcfg.c parseJSON.c cJSON.c ddsmsgqueue.c gtkWindow.c
ENDARGS=-lrt -lm `pkg-config --cflags --libs gtk+-3.0 webkitgtk-3.0` 
OBJS=$(SRCS:.c=.o)
all : $(OBJS)
	$(CC) $(OBJS) -o ../$(BUILDDIR)/$(OUT) $(ENDARGS)
	make clean

$(OBJS) : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

clean : 
	rm -rf $(OBJS)

