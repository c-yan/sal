CC = gcc
CFLAGS = -Os -Wall -mcpu=i586 -march=i586 -pipe -mno-cygwin
# -fno-exceptions
OBJECTS = sal.o
#EXTRA = -DPERIOD -lwinmm

all: sal.exe

sal.exe: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $< -mwindows $(EXTRA)
	strip -s $@

sal.o: sal.c
	$(CC) $(CFLAGS) -c -o $@ $<

sal.c: sal.h