CC=/opt/local/bin/clang-mp-3.5

GUNROCK_DIR=/Users/aterrel/workspace/apps/gunrock
GUNROCK_INC=$(GUNROCK_DIR)/gunrock
GUNROCK_LIBDIR=$(GUNROCK_DIR)/build/lib/
GUNROCK_LDFLAGS = -L$(GUNROCK_LIBDIR) -Wl,-rpath -Wl,$(GUNROCK_LIBDIR) -lgunrock

CFLAGS=-I. -I../../ -I../../../ -I$(GUNROCK_INC) -g -O0 -DUSE_PTHREADS
LDFLAGS=$(GUNROCK_LDFLAGS) -lpthread
DEPS = gunrock_clusters.h
OBJ = gunrock_clusters.o test_gunrock.o

.PHONY = help test

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

help:
	@echo 'Makefile for a webgraph etl                                            '
	@echo '                                                                       '
	@echo 'Usage:                                                                 '
	@echo '   make test                        tests the gunrock clusterings      '
	@echo '   make test_gunrock                build the gunrock object           '
	@echo '   make clean                       deletes temporary files            '
	@echo '                                                                       '


test: test_gunrock
	./test_gunrock

test_gunrock: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

make clean:
	rm -f $(OBJ) test_gunrock
