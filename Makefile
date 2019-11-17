CC = gcc
OFLAGS = -o
LFLAGS = -fPIC -shared -ldl
 
netlib.so: netlib.c
	$(CC) $(OFLAGS) netlib.so netlib.c $(LFLAGS)

rrnet.o: rrnet.c
	$(CC) -c $(OFLAGS) rrnet.o rrnet.c

monitor.o: monitor.c
	$(CC) -c $(OFLAGS) monitor.o monitor.c

monitor: rrnet.o monitor.o
	$(CC) $(OFLAGS) monitor rrnet.o monitor.o

clean:
	rm -rf *.o *.so *.out
