compiler: compiler.cpp parser.o lexer.o
	g++ -g -o compiler compiler.cpp parser.o lexer.o

parser.o: parser.cpp
	g++ -g -c parser.cpp

lexer.o: lexer.lex
	flex -o lexer.yy.cpp lexer.lex 
	g++ -g -c -lfl lexer.yy.cpp -o lexer.o

clean:
	rm -f lexer.yy.cpp lexer.o parser.o compiler

.PHONY: clean
