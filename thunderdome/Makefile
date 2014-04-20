# The worst makefile in the world!

JULIA_PATH = /home/jeff/src/julia2/julia

DEBUG = -g

default: all

all: example_client

libtd.so: td.h td.c
	$(CC) $(DEBUG) -fPIC -c td.c -o td.o
	$(CC) $(DEBUG) -shared -fPIC td.o -o $@ -ldl

libtd_julia.so: td.h td_julia.c libtd.so
	$(CC) $(DEBUG) -fPIC -c td_julia.c -o td_julia.o -I$(JULIA_PATH)/src -I$(JULIA_PATH)/src/support -I$(JULIA_PATH)/usr/include
	$(CC) $(DEBUG) -shared -fPIC td_julia.o -o $@ $(JULIA_PATH)/usr/lib/libjulia.so ./libtd.so

example_client: example_client.c td.h libtd_julia.so
	$(CC) $(DEBUG) $< -o $@ ./libtd.so

clean:
	rm -f *.so *.o
	rm -f example_client
