run: build
	./main 2> ./text.ll
	llvm-as ./text.ll -o ./bytecode.bc
	llc ./bytecode.bc
	gcc bytecode.s -o run

build:
	g++ -lLLVM main.cpp -o main

clean:
	rm -f run bytecode.bc bytecode.s main text.ll

.PHONY: .build
