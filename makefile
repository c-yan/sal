CC = gcc
CFLAGS = -Os -Wall -pipe #-march=i586 -fomit-frame-pointer 
OBJECTS = sal.o
EXTRA = -DPERIOD -lwinmm
#CRT = D:\bin\cl\mingw\lib\crt_noglob.o

all: sal.exe

sal.exe: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(CRT) $< -mwindows $(EXTRA)
	strip -s $@

sal.o: sal.c
	$(CC) $(CFLAGS) -c -o $@ $<

sal.c: sal.h
