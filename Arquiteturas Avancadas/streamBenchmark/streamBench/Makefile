CC = gcc
CFLAGS = -O2

#FF = g77
#FFLAGS = -O2

all: stream_c.exe

stream_f.exe: stream.f mysecond.o
	$(CC) $(CFLAGS) -c mysecond.c

stream_c.exe: stream.c
	$(CC) $(CFLAGS) stream.c -o stream_c.exe

clean:
	rm -f stream_c.exe *.o
