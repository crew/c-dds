
OUT=run
PKGS=`pkg-config --cflags --libs gtk+-3.0 webkitgtk-3.0 glib-2.0 `
CFLAGS=-c $(PKGS)
BUILDDIR=build
SHSRCS=dds_globals.c dict.c readcfg.c parseJSON.c cJSON.c gtkWindow.c dds_io.c dds_sem.c dds_slides.c
SRCS=main.c
ENDARGS=-lrt -lm -lc $(PKGS)
OBJS=$(SRCS:.c=.o)
SHOBJS=$(SHSRCS:.c=.o)
INCLUDEPY=-I/usr/include/python2.7 -lpython2.7
DEFCONFIGPATH=-DCONFIG_PATH=\"${PWD}/../Configs/PIE.conf\"
DEFKEYPATH=-DKEY_PATH=\"$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/main.c\"
ARM_HUH=$($(lscpu) | grep arm)
ifneq ($(ARM_HUH),)
	ARCH= -march=armv6zk -mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp
else
	ARCH=
endif
	
CC=gcc $(INCLUDEPY) $(DEFCONFIGPATH) $(DEFKEYPATH) $(ARCH)



all : $(OBJS)
	make shared
	$(CC) -L../$(BUILDDIR) $(OBJS) -o ../$(BUILDDIR)/$(OUT) -ldds $(ENDARGS)
	#$(CC) $(OBJS) -o ../$(BUILDDIR)/$(OUT) $(ENDARGS)
	make clean
shared : $(SHOBJS)
	$(CC) $(SHOBJS) -shared -o ../$(BUILDDIR)/libdds.so $(ENDARGS)

debug : debug_obj
	make debug_shared
	$(CC) -L../$(BUILDDIR) $(OBJS) -gdwarf-3 -g3 -o ../$(BUILDDIR)/$(OUT) -ldds $(ENDARGS)
	make clean
	#$(CC) $(OBJS) -g -o ../$(BUILDDIR)/$(OUT) $(ENDARGS)

debug_shared : debug_shobj
	$(CC) $(SHOBJS) -shared -g -o ../$(BUILDDIR)/libdds.so $(ENDARGS)

debug_obj : $(SRCS)
	$(CC) -gdwarf-3 -g3 $(CFLAGS) $(SRCS)

debug_shobj : $(SHSRCS)
	$(CC) -fPIC -gdwarf-3 -g3 $(CFLAGS) $(SHSRCS)

$(OBJS) : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

$(SHOBJS) : $(SHSRCS)
	$(CC) -fPIC $(CFLAGS) $(SHSRCS)

clean : 
	rm -rf $(OBJS) $(SHOBJS) dds_msg_queue.o
	rm -rf *.h.gch

tests :
	../tests/run-tests
