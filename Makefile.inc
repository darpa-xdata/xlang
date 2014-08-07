###############################################################################
## Common variables for all makefiles.                                       ##
##                                                                           ##
## Feel free to change this file, but to avoid local copies being checked    ##
## into the common repository, it is included in the gitignore file.         ##
##                                                                           ##
## To push changes to the common xlang repository, please force add:         ##
##     git add -f Makefile.inc                                               ##
###############################################################################

.PHONY = help test all clean

ARCH="win32"
LIBTYPE="dll"
CC = gcc
UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        ARCH=linux
        LIBTYPE=so
        SERVER_DIR=amd64/server
    endif
    ifeq ($(UNAME_S),Darwin)
        ARCH=darwin
        LIBTYPE=dylib
        SERVER_DIR=server
	CC=clang -std=c11 -g -O0
    endif
DEBUG = -g -O0

JULIA_BIN = $(shell dirname `which julia`)
JULIA_PATH = $(JULIA_BIN)/../..
JULIA_LIBDIR = $(JULIA_PATH)/usr/lib
JULIA_CFLAGS = -I$(JULIA_PATH)/src -I$(JULIA_PATH)/src/support -I$(JULIA_PATH)/include/julia -I$(JULIA_PATH)/usr/include
JULIA_LDFLAGS = -L$(JULIA_LIBDIR) -Wl,-rpath -Wl,$(JULIA_LIBDIR) -ljulia

PYTHON_EXE = $(shell python-config --exec-prefix)/bin/python
PYTHON_CFLAGS = $(shell python-config --cflags) \
               -I$(shell python -c "import numpy; print(numpy.get_include())") \
               -I$(shell python -c "import numpy; print(numpy.get_numarray_include())")
PYTHON_LIBDIR = $(shell python-config --prefix)/lib
PYTHON_LDFLAGS =-L$(PYTHON_LIBDIR) -Wl,-rpath -Wl,$(PYTHON_LIBDIR) $(shell python-config --ldflags)

JAVA_PATH = $JAVA_HOME/lib

# R parameters.
R_PATH = $(shell R RHOME)
R_CXX = $(shell $(R_PATH)/bin/R CMD config CXX)

RBLAS := 		$(shell $(R_PATH)/bin/R CMD config BLAS_LIBS)
RLAPACK := 		$(shell $(R_PATH)/bin/R CMD config LAPACK_LIBS)

R_RCPPINCL = $(shell echo 'Rcpp:::CxxFlags()' | $(R_PATH)/bin/R --vanilla --slave)
R_RINSIDEINCL = $(shell echo 'RInside:::CxxFlags()' | $(R_PATH)/bin/R --vanilla --slave)

R_RCPPLIBS = $(shell echo 'Rcpp:::LdFlags()'  | $(R_PATH)/bin/R --vanilla --slave)
R_RINSIDELIBS = $(shell echo 'RInside:::LdFlags()'  | $(R_PATH)/bin/R --vanilla --slave)

R_INCL = $(shell $(R_PATH)/bin/R CMD config --cppflags)
R_LDFLAGS = $(shell $(R_PATH)/bin/R CMD config --ldflags)

R_INCLUDES = $(R_INCL) $(R_RCPPINCL) $(R_RINSIDEINCL)
R_LIBS = $(R_LDFLAGS) $(R_RCPPLIBS) $(R_RINSIDELIBS)
CXX = $(shell $(R_HOME)/bin/R CMD config CXX)


GUNROCK_DIR=/Users/aterrel/workspace/apps/gunrock
GUNROCK_INC=$(GUNROCK_DIR)/gunrock
GUNROCK_LIBDIR=$(GUNROCK_DIR)/build/lib/
GUNROCK_LDFLAGS = -L$(GUNROCK_LIBDIR) -Wl,-rpath -Wl,$(GUNROCK_LIBDIR) -lgunrock

TD_DIR = /Users/aterrel/workspace/apps/xdata/xlang/thunderdome
TD_CFLAGS=-I$(TD_DIR)
TD_LDFLAGS = -L$(TD_DIR) -Wl,-rpath -Wl,$(TD_DIR) -ltd -ltd_python
