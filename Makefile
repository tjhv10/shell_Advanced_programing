CC = gcc
CFLAGS = -g -Wall

all: shell 

shell: shell.o
	$(CC) $(CFLAGS) -o shell shell.o

shell.o: shell.c 
	$(CC) $(CFLAGS) -c shell.c

clean:
	rm -f *.o  shell