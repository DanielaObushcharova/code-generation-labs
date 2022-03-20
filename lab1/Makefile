all: build test

build:
	g++ -c -I$$(gcc -print-file-name=plugin)/include -o gimple_print.o -fPIC gimple_print.cpp
	g++ -shared -o gimple_print.so gimple_print.o

test:
	gcc -fplugin=./gimple_print.so test.c -o test

clean:
	rm -f gimple_print.o gimple_print.so test

.PHONY: all build test clean
