CC = gcc
CFLAGS = -c -Wall -pedantic
STAND:=  -std=gnu99

all: output

output: laba_unix.o main.o 
	$(CC) laba_unix.o main.o -o laba6 -ludev
	
laba_unix.o: laba_unix.c
	$(CC) $(CFLAGS) $(STAND) laba_unix.c 

main.o: main.c
	$(CC) $(CFLAGS) $(STAND) main.c 
	
