CC = gcc
CFLAGS = -Os -Wall -pipe #-march=i586
OBJECTS = sal.o
#EXTRA = -DPERIOD -lwinmm

all: sal.exe

sal.exe: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $< -mwindows $(EXTRA)
	strip -s $@

sal.o: sal.c
	$(CC) $(CFLAGS) -c -o $@ $<

sal.c: sal.h
