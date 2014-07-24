CC=gcc
CFLAGS=-I. -I../../
DEPS = c_import.h
OBJ = c_import.o test_c_import.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test: test_c_import ../data/web-NotreDame.txt
	./test_c_import ../data/web-NotreDame.txt SNAP

test_c_import: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

../data/web-NotreDame.txt: ../data/web-NotreDame.txt.gz
	gunzip --keep $^
