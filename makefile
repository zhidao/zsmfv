CC=gcc
CFLAGS=-ansi -Wall -O3

SRC=zsmfv.c
TARGET=zsmfv

all: $(TARGET)
%: %.c
	$(CC) $(CFLAGS) -o $@ $?
clean:
	rm -f *.o *~ $(TARGET)
