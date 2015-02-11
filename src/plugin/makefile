BUILDDIR=../build/
SRC=dds_plugins.c dds_plugin_api.c
INCLUDES= -I.. -I/usr/include/lua5.1
OBJS=$(SRC:.c=.o)
CFLAGS=-c $(INCLUDES)
CC=clang
DEFS=-DPLUG_FOLDR=\"${PWD}/../../Plugins/\" -DPLUG_API_LUA=\"${PWD}/../../Plugins/dds_lib.lua\"
all : $(SRC)
	$(CC) $(CFLAGS) $(DEFS)  $(SRC) -pthread
	mv *.o $(BUILDDIR)

