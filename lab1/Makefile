COMPILE := g++ -c -I$$(gcc -print-file-name=plugin)/include -fPIC -fno-rtti  
SRC := ./src
OUT := ./out

all: build test

build:
	mkdir -p $(OUT)
	$(COMPILE) -o $(OUT)/gimple_print.o $(SRC)/gimple_print.cpp
	$(COMPILE) -o $(OUT)/bb_info_collector.o $(SRC)/bb_info_collector.cpp
	g++ -shared -o $(OUT)/gimple_print.so $(OUT)/gimple_print.o $(OUT)/bb_info_collector.o
	rm $(OUT)/gimple_print.o $(OUT)/bb_info_collector.o

test:
	mkdir -p $(OUT)
	gcc -fplugin=$(OUT)/gimple_print.so -o $(OUT)/test $(SRC)/test.c

clean:
	rm -f $(OUT)

.PHONY: all build test clean
