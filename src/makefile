CC=gcc -DKEY_PATH=\"$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/main.c\"
OUT=run
PKGS=`pkg-config --cflags --libs gtk+-3.0 webkitgtk-3.0 glib-2.0`
CFLAGS=-c $(PKGS)
BUILDDIR=build
SRCS=main.c dds_globals.c dict.c readcfg.c parseJSON.c cJSON.c gtkWindow.c dds_io.c dds_sem.c dds_slides.c
ENDARGS=-lrt -lm -lc $(PKGS)
OBJS=$(SRCS:.c=.o)
all : $(OBJS)
	$(CC) $(OBJS) -o ../$(BUILDDIR)/$(OUT) $(ENDARGS)
	make clean
debug : debug_obj
	$(CC) $(OBJS) -g -o ../$(BUILDDIR)/$(OUT) $(ENDARGS)

debug_obj : $(SRCS)
	$(CC) -gdwarf-3 -g3 $(CFLAGS) $(SRCS)

$(OBJS) : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

clean : 
	rm -rf $(OBJS) ddsmsgqueue.o
	rm -rf *.h.gch

tests :
	../tests/run-tests