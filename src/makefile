
OUT=run
PKGS=`pkg-config --cflags --libs gtk+-3.0 webkitgtk-3.0 glib-2.0 python`
CFLAGS=-c $(PKGS)
BUILDDIR=build
PYSHSRCS=plugin/dds_piobj_list.c plugin/dds_plugins.c plugin/dds_python_helpers.c
PYMODSETUP=setup.py
PYMODOUT=$(wildcard plugin/build/lib.*)
SHSRCSLOC=dds_globals.c dict.c readcfg.c parseJSON.c cJSON.c gtkWindow.c dds_io.c dds_sem.c dds_slides.c
SHSRCS=$(SHSRCSLOC) $(PYSHSRCS)
SRCS=main.c
ENDARGS=-lrt -lm -lc $(PKGS)
OBJS=$(SRCS:.c=.o)
SHOBJS=$(SHSRCSLOC:.c=.o) dds_piobj_list.o dds_plugins.o dds_python_helpers.o
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
	cd plugin && python $(PYMODSETUP) build_ext && cd ..
	#TODO: Find a better fix than this
	make copymodule 
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

#TODO: See above; this is gross
copymodule : $(PYMODOUT)

.PHONY : force

$(PYMODOUT) : force
	cp $@/dds.so ../Plugins

clean : 
	rm -rf $(OBJS) $(SHOBJS) dds_msg_queue.o
	rm -rf *.h.gch

tests :
	../tests/run-tests
