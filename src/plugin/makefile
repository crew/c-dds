BUILDDIR=../build/plugins
SRC=dds_plugins.c
INCLUDES= -I.. -I/usr/include/lua5.1
OBJS=$(SRC:.c=.o)
CFLAGS=-c $(INCLUDES)
CC=clang
DEFS=-DPLUG_FOLDR=\"${PWD}/../../Plugins/\"
all : $(SRC)
	$(CC) $(CFLAGS) $(DEFS)  $(SRC) -o $(BUILDDIR)

