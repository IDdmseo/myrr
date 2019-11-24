$(CC)= gcc
$(SFLAGS)= -fPIC -shared

rr_library.so: rrnet.c netlib.c
	$(CC) $(SFLAGS) rrnet.c netlib.c -ldl -lpthread

monitor: monitor.c rrnet.c
	$(CC) -o monitor monitor.c rrnet.c -lpthread

clean:
	rm -rf *.o *.swp *.so
