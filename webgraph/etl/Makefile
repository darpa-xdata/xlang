include ../../Makefile.inc


CFLAGS += -I. -I../../
DEPS = c_import.h
OBJ = c_import.o test_c_import.o

.PHONY = help test

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

help:
	@echo 'Makefile for a webgraph etl                                            '
	@echo '                                                                       '
	@echo 'Usage:                                                                 '
	@echo '   make test                        tests the c import etl function    '
	@echo '   make c_import.o                  build the c import etl object      '
	@echo '   make clean                       deletes temporary files            '
	@echo '                                                                       '


test: test_c_import ../data/web-NotreDame.txt ../data/pld-index-sample ../data/pld-arc-sample
	./test_c_import SNAP ../data/web-NotreDame.txt
	./test_c_import WDC ../data/pld-arc-sample ../data/pld-index-sample

test_c_import: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

../data/pld-index-sample: ../data/pld-index-sample.gz
	gunzip --keep $^

../data/pld-arc-sample: ../data/pld-arc-sample.gz
	gunzip --keep $^

../data/web-NotreDame.txt: ../data/web-NotreDame.txt.gz
	gunzip --keep $^

make clean:
	rm -f $(OBJ) test_c_import ../data/web-NotreDame.txt ../data/pld-arc-sample ../data/pld-index-sample
