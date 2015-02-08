
OUT=run
PKGS=`pkg-config --cflags --libs gtk+-3.0 webkitgtk-3.0 glib-2.0 `
CFLAGS=-c $(PKGS)
BUILDDIR=build
SHSRCS= 
SRCS=main.c dds_globals.c dict.c readcfg.c parseJSON.c cJSON.c gtkWindow.c dds_io.c dds_sem.c dds_slides.c
ENDARGS=-lrt -lm -lc $(PKGS)
OBJS=$(SRCS:.c=.o)
SHOBJS=$(SHSRCS:.c=.o)
INCLUDEPY=-I/usr/include/lua5.1 -I/usr/include/python2.7 -lpython2.7 -llua5.1
DEFCONFIGPATH=-DCONFIG_PATH=\"${PWD}/../Configs/PIE.conf\"
DEFKEYPATH=-DKEY_PATH=\"$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))/main.c\"
PLUGSRC=plugin
COREOBJ=core.o
ARM_HUH=$($(lscpu) | grep arm)
ifneq ($(ARM_HUH),)
	ARCH= -march=armv6zk -mcpu=arm1176jzf-s -mfloat-abi=hard -mfpu=vfp
else
	ARCH=
endif
	
CC=clang -std=gnu11 -Wno-unused-parameter $(INCLUDEPY) $(DEFCONFIGPATH) $(DEFKEYPATH) $(ARCH)



all : 
	make clean
	make $(BUILDDIR)
	make core	
	make plugins
	$(CC) -I../$(BUILDDIR) $(BUILDDIR)/* -o $(BUILDDIR)/$(OUT) $(ENDARGS)

shared : $(SHOBJS)
	$(CC) $(SHOBJS) -shared -o $(BUILDDIR)/libdds.so $(ENDARGS)

plugins :
	cd $(PLUGSRC) && make
	#$(MAKE) -C $(PLUGSRC)

debug : debug_obj
	make debug_shared
	$(CC) -L../$(BUILDDIR) $(OBJS) -gdwarf-3 -g3 -o $(BUILDDIR)/$(OUT) -ldds $(ENDARGS)
	make clean

debug_shared : debug_shobj
	$(CC) $(SHOBJS) -shared -g -o $(BUILDDIR)/libdds.so $(ENDARGS)

debug_obj : $(SRCS)
	$(CC) -gdwarf-3 -g3 $(CFLAGS) $(SRCS)

debug_shobj : $(SHSRCS)
	$(CC) -fPIC -gdwarf-3 -g3 $(CFLAGS) $(SHSRCS)

core : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)
	ld -r $(OBJS) -o $(BUILDDIR)/$(COREOBJ)
	rm $(OBJS)

$(SHOBJS) : $(SHSRCS)
	$(CC) -fPIC $(CFLAGS) $(SHSRCS)

$(BUILDDIR) :
	mkdir $(BUILDDIR)

clean : 
	rm -rf build/
	rm -rf $(OBJS) $(SHOBJS) dds_msg_queue.o
	rm -rf *.h.gch

tests :
	../tests/run-tests
