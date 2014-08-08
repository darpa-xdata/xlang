include ../../../Makefile.inc

DEPS = ../../etl/c_import.o ../../cluster/gunrock/gunrock_clusters.o
OBJ = c-loader.o

CFLAGS += -I. -I../../ -I../../../ $(TD_CFLAGS) -I$(GUNROCK_INC) -DTD_PYTHON_EXE="\"$(PYTHON_EXE)\"" -DTD_HAS_PYTHON=1 -DTD_DIR="\"$(TD_DIR)\""
LDFLAGS += $(GUNROCK_LDFLAGS)  $(TD_LDFLAGS)


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

app: c-loader

c-loader: $(OBJ) $(DEPS)
	$(CC) $(CFLAGS) $(LDFLAGS)  $^  -o $@

../../data/pld-index-sample: ../../data/pld-index-sample.gz
	gunzip --keep $^

../../data/pld-arc-sample: ../../data/pld-arc-sample.gz
	gunzip --keep $^

../../data/web-NotreDame.txt: ../../data/web-NotreDame.txt.gz
	gunzip --keep $^

clean:
	rm -f $(OBJ) c-loader
