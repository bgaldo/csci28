# Makefile for ulast.c

CC     = cc 
CFLAGS = -Wall -Wextra -g

ulast: ulast.o utmplib.o
	$(CC) $(CFLAGS) -o ulast ulast.o utmplib.o

utmplib.o: utmplib.c
	$(CC) $(CFLAGS) -c utmplib.c

ulast.o: ulast.c
	$(CC) $(CFLAGS) -c ulast.c

clean: rm -f ulast utmplib *.o
