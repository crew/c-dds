
OUT=run
PKGS=`pkg-config --cflags --libs gtk+-3.0 webkitgtk-3.0 glib-2.0 `
CFLAGS=-c $(PKGS)
BUILDDIR=build
SRCS=main.c dds_globals.c dict.c readcfg.c parseJSON.c cJSON.c gtkWindow.c dds_io.c dds_sem.c dds_slides.c
ENDARGS=-lrt -lm -lc $(PKGS)
OBJS=$(SRCS:.c=.o)
INCLUDEPY=-I/usr/include/python2.7 -lpython2.7
DEFCONFIGPATH=\"${PWD}/../Configs/PIE.conf\"
DEFKEYPATH=\"$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/main.c\"
ARM_HUH=$($(lscpu) | grep arm)
ifneq ($(ARM_HUH),)
	ARCH= -march=armv6zk -mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp
else
	ARCH=
endif
	
CC=gcc $(INCLUDEPY) $(DEFCONFIGPATH) $(DEFKEYPATH) $(ARCH)



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
	rm -rf $(OBJS) dds_msg_queue.o
	rm -rf *.h.gch

tests :
	../tests/run-tests
