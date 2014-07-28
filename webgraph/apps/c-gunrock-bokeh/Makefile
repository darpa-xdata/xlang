DEPS = ../../etl/c_import.o ../../cluster/gunrock/gunrock_clusters.o
OBJ = app-c-gunrock-bokeh.o

.PHONY = help test

GUNROCK_DIR=/Users/aterrel/workspace/apps/gunrock
GUNROCK_INC=$(GUNROCK_DIR)/gunrock
GUNROCK_LIBDIR=$(GUNROCK_DIR)/build/lib/
GUNROCK_LDFLAGS = -L$(GUNROCK_LIBDIR) -Wl,-rpath -Wl,$(GUNROCK_LIBDIR) -lgunrock

PYTHON_EXE = $(shell python-config --exec-prefix)/bin/python
PYTHON_CFLAGS = $(shell python-config --cflags) \
               -I$(shell python -c "import numpy; print(numpy.get_include())") \
               -I$(shell python -c "import numpy; print(numpy.get_numarray_include())")
PYTHON_LIBDIR = $(shell python-config --prefix)/lib
PYTHON_LDFLAGS =-L$(PYTHON_LIBDIR) -Wl,-rpath -Wl,$(PYTHON_LIBDIR) $(shell python-config --ldflags)

TD_DIR = /Users/aterrel/workspace/apps/xdata/xlang/thunderdome
TD_CFLAGS=-I$(TD_DIR)
TD_LDFLAGS = -L$(TD_DIR) -Wl,-rpath -Wl,$(TD_DIR) -ltd -ltd_python

CC := /opt/local/bin/clang-mp-3.5 -g -O0
CFLAGS := -I. -I../../ -I../../../ $(TD_CFLAGS) -I$(GUNROCK_INC) -DTD_PYTHON_EXE="\"$(PYTHON_EXE)\"" -DTD_HAS_PYTHON=1 -DTD_DIR="\"$(TD_DIR)\""
LDFLAGS=$(GUNROCK_LDFLAGS)  $(TD_LDFLAGS)


%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

help:
	@echo 'Makefile for a webgraph etl                                            '
	@echo '                                                                       '
	@echo 'Usage:                                                                 '
	@echo '   make test                        tests app on SNAP and WDC test sets'
	@echo '   make app                         makes the app'
	@echo '   make clean                       deletes temporary files            '
	@echo '                                                                       '


test: app ../../data/web-NotreDame.txt ../../data/pld-index-sample ../../data/pld-arc-sample
	time ./app 4 WDC ../../data/pld-arc-sample ../../data/pld-index-sample
	time ./app 10 SNAP ../../data/web-NotreDame.txt

app: $(OBJ) $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS)  $^  -o $@

../../data/pld-index-sample: ../../data/pld-index-sample.gz
	gunzip --keep $^

../../data/pld-arc-sample: ../../data/pld-arc-sample.gz
	gunzip --keep $^

../../data/web-NotreDame.txt: ../../data/web-NotreDame.txt.gz
	gunzip --keep $^

make clean:
	rm -f $(OBJ) app ../../data/web-NotreDame.txt ../../data/pld-arc-sample ../../data/pld-index-sample
