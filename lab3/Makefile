run: compiler
	./compiler < input.txt 2> ./text.ll
	llvm-as ./text.ll -o ./bytecode.bc
	llc ./bytecode.bc
	gcc bytecode.s -o run

compiler: compiler.cpp parser.o lexer.o
	g++ -lLLVM -g -o compiler compiler.cpp parser.o lexer.o

parser.o: parser.cpp
	g++ -g -c parser.cpp

lexer.o: lexer.lex
	flex -o lexer.yy.cpp lexer.lex 
	g++ -g -c -lfl lexer.yy.cpp -o lexer.o

clean:
	rm -f lexer.yy.cpp lexer.o parser.o compiler run bytecode.bc bytecode.s text.ll

.PHONY: clean run
