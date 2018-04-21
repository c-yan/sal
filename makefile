CC = gcc
CFLAGS = -Os -Wall -fno-exceptions -pipe
LIBS = -lwinmm
OBJECTS = sal.o

all: sal.exe

sal.exe: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $< -mwindows $(LIBS)
	strip -s $@

sal.o: sal.c
	$(CC) $(CFLAGS) -c -o $@ $<

sal.c: sal.h
