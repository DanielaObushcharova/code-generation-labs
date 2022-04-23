compiler: parser.o lexer.o
	g++ -g -o compiler parser.o lexer.o

parser.o: parser.cpp lexer.o
	g++ -g -c parser.cpp

lexer.o: lexer.lex
	flex -o lexer.yy.cpp lexer.lex 
	g++ -g -c -lfl lexer.yy.cpp -o lexer.o

clean:
	rm -f lexer.yy.cpp lexer.o parser.o compiler

.PHONY: clean
