CC=gcc
OUT=run
all : test

test : *.c
	$(CC) *.c -o $(OUT) -ggdb
