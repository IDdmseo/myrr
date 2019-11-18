CC = gcc
OFLAGS = -o
LFLAGS = -fPIC -shared -ldl
IFLAGS = -Iinclude
 
netlib.so: netlib.c
	$(CC) $(OFLAGS) netlib.so netlib.c $(LFLAGS) $(IFLAGS)

rrnet.o: rrnet.c
	$(CC) -c $(OFLAGS) rrnet.c -lpthread

monitor.o: monitor.c
	$(CC) -c $(OFLAGS) monitor.c

monitor: rrnet.o monitor.o
	$(CC) $(OFLAGS) monitor rrnet.o monitor.o

clean:
	rm -rf *.o *.so *.out
