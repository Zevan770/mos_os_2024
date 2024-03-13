CC     := gcc
CFLAGS := -Wall -I./include


all: code/fibo.o code/main.o
	$(CC) $(CFLAGS) -o fibo code/fibo.o code/main.o

code/fibo.o: code/fibo.c include/fibo.h
	$(CC) $(CFLAGS) -c code/fibo.c -o code/fibo.o

code/main.o: code/main.c
	$(CC) $(CFLAGS) -c code/main.c -o code/main.o

.PHONY: all clean
clean:
	rm -f code/*.o 
