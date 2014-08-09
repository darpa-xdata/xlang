include ../../../Makefile.inc

CXXFLAGS += -I. -I../../ -I../../../
DEPS =
OBJ = app-c-fielder-igraph.o ../../etl/c_import.o

.PHONY = help test


help:
	@echo 'Makefile for a webgraph etl                                            '
	@echo '                                                                       '
	@echo 'Usage:                                                                 '
	@echo '   make test                        tests the c import etl function    '
	@echo '   make app                         makes the app'
	@echo '   make clean                       deletes temporary files            '
	@echo '                                                                       '


test: app ../../data/web-NotreDame.txt ../../data/pld-index-sample ../../data/pld-arc-sample
	./app 10 SNAP ../../data/web-NotreDame.txt
	./app 4 WDC ../../data/pld-arc-sample ../../data/pld-index-sample

app: $(OBJ)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $^ $(LDLIBS) -o $@

../../data/pld-index-sample: ../../data/pld-index-sample.gz
	gunzip --keep $^

../../data/pld-arc-sample: ../../data/pld-arc-sample.gz
	gunzip --keep $^

../../data/web-NotreDame.txt: ../../data/web-NotreDame.txt.gz
	gunzip --keep $^

make clean:
	rm -f $(OBJ) app ../../data/web-NotreDame.txt ../../data/pld-arc-sample ../../data/pld-index-sample
