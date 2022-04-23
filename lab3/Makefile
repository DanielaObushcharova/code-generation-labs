run: build
	./lexer < input.txt

build:
	flex -o lexer.yy.cpp lexer.lex 
	g++ -lfl lexer.yy.cpp -o lexer

clean:
	rm -f lexer.yy.cpp lexer

.PHONY: build clean
