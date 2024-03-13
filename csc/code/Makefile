CC     := gcc
CFLAGS := -Wall -I../include

.PHONY: all clean

all: fibo.o main.o

fibo.o: fibo.c ../include/fibo.h
	$(CC) $(CFLAGS) -c fibo.c -o fibo.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

clean:
	rm -f *.o
