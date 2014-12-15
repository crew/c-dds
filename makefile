CC=gcc
OUT=run
CFLAGS=-c
SRCS=main.c dict.c
OBJS=$(SRCS:.c=.o)
all : $(OBJS)
	$(CC) $(OBJS) -o $(OUT)
	make clean

$(OBJS) : $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)

clean : 
	rm -rf $(OBJS)

