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
## comment this out if you need a different version of R, 
## and set set R_HOME accordingly as an environment variable
R_HOME := 		$(shell R RHOME)

## include headers and libraries for R 
RCPPFLAGS := 		$(shell $(R_HOME)/bin/R CMD config --cppflags)
RLDFLAGS := 		$(shell $(R_HOME)/bin/R CMD config --ldflags)
RBLAS := 		$(shell $(R_HOME)/bin/R CMD config BLAS_LIBS)
RLAPACK := 		$(shell $(R_HOME)/bin/R CMD config LAPACK_LIBS)

## if you need to set an rpath to R itself, also uncomment
#RRPATH :=		-Wl,-rpath,$(R_HOME)/lib

## include headers and libraries for Rcpp interface classes
## note that RCPPLIBS will be empty with Rcpp (>= 0.11.0) and can be omitted
RCPPINCL := 		$(shell echo 'Rcpp:::CxxFlags()' | $(R_HOME)/bin/R --vanilla --slave)
RCPPLIBS := 		$(shell echo 'Rcpp:::LdFlags()'  | $(R_HOME)/bin/R --vanilla --slave)

## include headers and libraries for RInside embedding classes
RINSIDEINCL := 		$(shell echo 'RInside:::CxxFlags()' | $(R_HOME)/bin/R --vanilla --slave)
RINSIDELIBS := 		$(shell echo 'RInside:::LdFlags()'  | $(R_HOME)/bin/R --vanilla --slave)

## compiler etc settings used in default make rules
CXX := 			$(shell $(R_HOME)/bin/R CMD config CXX)
CPPFLAGS := 		-Wall $(shell $(R_HOME)/bin/R CMD config CPPFLAGS) $(TD_CFLAGS) 
CXXFLAGS := 		$(RCPPFLAGS) $(RCPPINCL) $(RINSIDEINCL) $(shell $(R_HOME)/bin/R CMD config CXXFLAGS)
LDLIBS := 		$(RLDFLAGS) $(RRPATH) $(RBLAS) $(RLAPACK) $(RCPPLIBS) $(RINSIDELIBS)

GUNROCK_DIR=/Users/aterrel/workspace/apps/gunrock
GUNROCK_INC=$(GUNROCK_DIR)/gunrock
GUNROCK_LIBDIR=$(GUNROCK_DIR)/build/lib/
GUNROCK_LDFLAGS = -L$(GUNROCK_LIBDIR) -Wl,-rpath -Wl,$(GUNROCK_LIBDIR) -lgunrock

TD_DIR = /Users/aterrel/workspace/apps/xdata/xlang/thunderdome
TD_CFLAGS=-I$(TD_DIR)
TD_LDFLAGS = -L$(TD_DIR) -Wl,-rpath -Wl,$(TD_DIR) -ltd -ltd_python
