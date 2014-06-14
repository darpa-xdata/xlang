# The worst makefile in the world!

JULIA_PATH = /Applications/Julia-0.2.1.app/Contents/Resources/julia/


PYTHON_CFLAG = -I/Users/aterrel/workspace/opt/apps/anaconda/anaconda-1.9.1/anaconda/include/python2.7 \
               -I/Users/aterrel/workspace/opt/apps/anaconda/anaconda-1.9.1/anaconda/lib/python2.7/site-packages/numpy/core/include
PYTHON_LIBDIR = /Users/aterrel/workspace/opt/apps/anaconda/anaconda-1.9.1/anaconda/lib
PYTHON_LDFLAG =-L$(PYTHON_LIBDIR) -Wl,-rpath -Wl,$(PYTHON_LIBDIR) -lpython

JAVA_PATH = $JAVA_HOME/lib

DEBUG = -g

# R parameters.
R_PATH = $(shell R RHOME)
R_CXX = $(shell $(R_PATH)/bin/R CMD config CXX)

R_RCPPINCL = $(shell echo 'Rcpp:::CxxFlags()' | $(R_PATH)/bin/R --vanilla --slave)
R_RINSIDEINCL = $(shell echo 'RInside:::CxxFlags()' | $(R_PATH)/bin/R --vanilla --slave)

R_RCPPLIBS = $(shell echo 'Rcpp:::LdFlags()'  | $(R_PATH)/bin/R --vanilla --slave)
R_RINSIDELIBS = $(shell echo 'RInside:::LdFlags()'  | $(R_PATH)/bin/R --vanilla --slave)

R_INCL = $(shell $(R_PATH)/bin/R CMD config --cppflags)
R_LDFLAGS = $(shell $(R_PATH)/bin/R CMD config --ldflags)

R_INCLUDES = $(R_INCL) $(R_RCPPINCL) $(R_RINSIDEINCL)
R_LIBS = $(R_LDFLAGS) $(R_RCPPLIBS) $(R_RINSIDELIBS)
CC = gcc -v  
#CC = cc -I"$(JAVA_HOME)"/include -I"$(JAVA_HOME)"/include/win32

default: all

all: example_client ds_roundtrip

libtd.so: td.h td_ds.h td.c td_ds.c
	$(CC) $(DEBUG) -fPIC -c td.c -o td.o
	$(CC) $(DEBUG) -fPIC -c td_ds.c -o td_ds.o
	$(CC) $(DEBUG) -shared -fPIC td.o td_ds.o -o $@ -ldl

libtd_julia.so: td.h td_julia.c libtd.so
	$(CC) $(DEBUG) -fPIC -c td_julia.c -o td_julia.o -I$(JULIA_PATH)/src -I$(JULIA_PATH)/src/support -I$(JULIA_PATH)/include/julia
	$(CC) $(DEBUG) -shared -fPIC td_julia.o -o $@ $(JULIA_PATH)/usr/lib/libjulia.so ./libtd.so

libtd_r.so: td.h libtd.so
	$(R_CXX) $(DEBUG) -fPIC -c td_r.cpp -o td_r.o $(R_INCLUDES)
	$(R_CXX) $(DEBUG) -shared -fPIC -o $@ $(R_LDFLAGS) $(R_LIBS)
	
# for windows for now...? do we have to make a dll??
libtd_java.so: td.h td_java.c libtd.so
	$(CC) $(DEBUG) -D__int64=int64_t -fPIC -c td_java.c -o td_java.o -I"$(JAVA_HOME)"/include -I"$(JAVA_HOME)"/include/win32
#	$(CC) $(DEBUG) -fPIC -c td_java.c -o td_java.o -I"$(JAVA_HOME)"/include
	$(CC) $(DEBUG) -shared -fPIC td_java.o -o $@ "$(JAVA_HOME)"/jre/bin/server/jvm.dll ./libtd.so

example_client: example_client.c td.h libtd_julia.so libtd_python.so
	$(CC) $(DEBUG) -DTD_HAS_PYTHON=1 -DTD_HAS_JULIA=1 $< -o $@ ./libtd.so
	
example_java_client: example_client.c td.h libtd_java.so 
	$(CC) $(DEBUG) $< -o $@ ./libtd.so ./libtd_java.so "$(JAVA_HOME)"/jre/bin/server/jvm.dll
#	$(CC) $(DEBUG) -o $@ ./libtd.so ./libtd_java.so "$(JAVA_HOME)"/jre/bin/server/jvm.dll
	
example_java_client2: example_client.c td.h td_java.c
	$(CC) $(DEBUG) -D__int64=int64_t -fPIC -c td_java.c -o td_java.o -I"$(JAVA_HOME)"/include -I"$(JAVA_HOME)"/include/win32
	$(CC) $(DEBUG) $< -o $@ ./libtd.so 

simple: td.h td_ds.h td.c td_ds.c example_client.c td_java.c
#	$(CC) $(DEBUG) -ljvm -L$(JAVA_HOME)/jre/lib/server -rpath=$(JAVA_HOME)/jre/lib/server td.c td_ds.c example_client.c td_java.c -o simple -I"$(JAVA_HOME)"/include -I"$(JAVA_HOME)"/include/darwin "$(JAVA_HOME)"/jre/lib/server/libjvm.dylib 
	$(CC) $(DEBUG) -ljvm -L$(JAVA_HOME)/jre/lib/server  -rpath=$(JAVA_HOME)/jre/lib/server td.c td_ds.c example_client.c td_java.c -o simple -I"$(JAVA_HOME)"/include -I"$(JAVA_HOME)"/include/darwin "$(JAVA_HOME)"/jre/lib/server/libjvm.dylib 

#	$(CC) $(DEBUG) -ljvm -L$(JAVA_HOME)/jre/lib/server  -dynamiclib simple -install_name @rpath/t/libjvm.dylib -o libjvm.dylib
# ghc --make Main.hs mycbinding.o -ljvm -L<javahome>/jre/lib/server -optl-Wl,-rpath,<javahome>/jre/lib/server -o mybinary

ds_roundtrip: ds_roundtrip.c td.h

libtd_python.so: td.h td_python.c libtd.so
	$(CC) $(DEBUG) -fPIC -c td_python.c -o td_python.o $(PYTHON_CFLAG)
	$(CC) $(DEBUG) -shared -fPIC td_python.o -o $@ -I$(PYTHON_LDFLAG) ./libtd.so

example_python: example_client.c td.h libtd_python.so
	$(CC) $(DEBUG) -DTD_HAS_PYTHON=1 $< -o $@ ./libtd.so

clean:
	rm -f *.so *.o
	rm -f example_client
	rm -f ds_roundtrip
	rm -f simple
	