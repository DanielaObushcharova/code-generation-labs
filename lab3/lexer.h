#ifndef LEXER_H
#define LEXER_H

#include <iostream>

struct Position {
    int line, column, index;

    Position(int _line, int _column, int _index):
        line(_line), column(_column), index(_index) {}
};

struct Fragment {
    Position begin, end;

    Fragment(Position _begin, Position _end):
        begin(_begin), end(_end) {}
};

enum TokenType {
    NUMBER,
    IF,
    ELSE,
    OPEN_BRACE,
    CLOSE_BRACE,
    DELIMITER,
    WHILE,
    IDENT,
    OP,
    ASSIGN,
    RETURN,
    EOF_TOKEN,
};

struct Token {
    int numberAttr;
    std::string identAttr;
    char opAttr;
    TokenType type;
    Fragment fragment;

    Token(TokenType _type, Fragment _fragment):
        type(_type), fragment(_fragment) {}

    std::string toString();
};

std::ostream& operator<<(std::ostream &out, const TokenType &type);
std::ostream& operator<<(std::ostream &out, const Token &token);
std::ostream& operator<<(std::ostream &out, const Fragment &fragment);
std::ostream& operator<<(std::ostream &out, const Position &position);

Token yylex();
#endif
