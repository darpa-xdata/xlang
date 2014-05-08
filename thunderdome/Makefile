# The worst makefile in the world!

JULIA_PATH = /Applications/Julia-0.2.1.app/Contents/Resources/julia/


PYTHON_CFLAG = -I/Users/aterrel/workspace/opt/apps/anaconda/anaconda-1.9.1/anaconda/include/python2.7 \
               -I/Users/aterrel/workspace/opt/apps/anaconda/anaconda-1.9.1/anaconda/lib/python2.7/site-packages/numpy/core/include
PYTHON_LIBDIR = /Users/aterrel/workspace/opt/apps/anaconda/anaconda-1.9.1/anaconda/lib
PYTHON_LDFLAG =-L$(PYTHON_LIBDIR) -Wl,-rpath -Wl,$(PYTHON_LIBDIR) -lpython

DEBUG = -g

default: all

all: example_python

libtd.so: td.h td.c
	$(CC) $(DEBUG) -fPIC -c td.c -o td.o
	$(CC) $(DEBUG) -shared -fPIC td.o -o $@ -ldl

libtd_julia.so: td.h td_julia.c libtd.so
	$(CC) $(DEBUG) -fPIC -c td_julia.c -o td_julia.o -I$(JULIA_PATH)/src -I$(JULIA_PATH)/src/support -I$(JULIA_PATH)/include/julia
	$(CC) $(DEBUG) -shared -fPIC td_julia.o -o $@ $(JULIA_PATH)/usr/lib/libjulia.so ./libtd.so

example_client: example_client.c td.h libtd_julia.so
	$(CC) $(DEBUG) $< -o $@ ./libtd.so

libtd_python.so: td.h td_python.c libtd.so
	$(CC) $(DEBUG) -fPIC -c td_python.c -o td_python.o $(PYTHON_CFLAG)
	$(CC) $(DEBUG) -shared -fPIC td_python.o -o $@ -I$(PYTHON_LDFLAG) ./libtd.so

example_python: example_client.c td.h libtd_python.so
	$(CC) $(DEBUG) $< -o $@ ./libtd.so

clean:
	rm -f *.so *.o
	rm -f example_client
