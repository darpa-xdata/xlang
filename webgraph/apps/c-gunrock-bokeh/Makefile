TD_CFLAGS=-I. -I../../ -I../../../
DEPS = ../../etl/c_import.o ../../cluster/gunrock/gunrock_clusters.o
OBJ = app-c-gunrock-bokeh.o

.PHONY = help test

GUNROCK_DIR=/Users/aterrel/workspace/apps/gunrock
GUNROCK_INC=$(GUNROCK_DIR)/gunrock
GUNROCK_LIBDIR=$(GUNROCK_DIR)/build/lib/
GUNROCK_LDFLAGS = -L$(GUNROCK_LIBDIR) -Wl,-rpath -Wl,$(GUNROCK_LIBDIR) -lgunrock

CC := /opt/local/bin/clang-mp-3.5
CFLAGS := $(TD_CFLAGS) -I$(GUNROCK_INC)
LDFLAGS=$(GUNROCK_LDFLAGS)


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
	./app 4 WDC ../../data/pld-arc-sample ../../data/pld-index-sample
	./app 10 SNAP ../../data/web-NotreDame.txt

app: $(OBJ) $(DEPS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

../../data/pld-index-sample: ../../data/pld-index-sample.gz
	gunzip --keep $^

../../data/pld-arc-sample: ../../data/pld-arc-sample.gz
	gunzip --keep $^

../../data/web-NotreDame.txt: ../../data/web-NotreDame.txt.gz
	gunzip --keep $^

make clean:
	rm -f $(OBJ) app ../../data/web-NotreDame.txt ../../data/pld-arc-sample ../../data/pld-index-sample
