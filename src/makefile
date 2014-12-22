CC=gcc
OUT=run
PKGS=`pkg-config --cflags --libs gtk+-3.0 webkitgtk-3.0 glib-2.0`
CFLAGS=-c $(PKGS)
BUILDDIR=build
SRCS=main.c dict.c readcfg.c parseJSON.c cJSON.c ddsmsgqueue.c gtkWindow.c dds_io.c
ENDARGS=-lrt -lm  $(PKGS)
OBJS=$(SRCS:.c=.o)
all : $(OBJS)
	$(CC) $(OBJS) -o ../$(BUILDDIR)/$(OUT) $(ENDARGS)
	make clean

$(OBJS) : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

clean : 
	rm -rf $(OBJS)

