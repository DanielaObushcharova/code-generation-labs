%{
#include <iostream>
#include <vector>

extern "C" int yylex();

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
    };

    Position pos(1, 1, 0);
    Fragment cur(pos, pos);
    bool eof = false;

    void run_user_action() {
        cur.begin = pos;
        int sz = strlen(yytext);
        pos.index += sz;
        pos.column += sz;
        cur.end = pos;
    }

std::vector<Token> tokens;

#define YY_USER_ACTION { run_user_action(); }
%}
IDENT [a-zA-Z][a-zA-Z0-9]*
NUMBER -?(0|[1-9][0-9]*)
OP [*+-]
%%
"return" {
    Token token(RETURN, cur);
    tokens.push_back(token);
}
"if" {
    Token token(IF, cur);
    tokens.push_back(token);
}
"while" {
    Token token(WHILE, cur);
    tokens.push_back(token);
}
"else" {
    Token token(ELSE, cur);
    tokens.push_back(token);
}
{IDENT} {
    Token token(IDENT, cur);
    token.identAttr = yytext;
    tokens.push_back(token);
}
{NUMBER} {
    Token token(NUMBER, cur);
    token.numberAttr = std::stoi(yytext);
    tokens.push_back(token);
}
{OP} {
    Token token(NUMBER, cur);
    token.opAttr = yytext[0];
    tokens.push_back(token);
}
"{" {
    Token token(OPEN_BRACE, cur);
    tokens.push_back(token);
}
"}" {
    Token token(CLOSE_BRACE, cur);
    tokens.push_back(token);
}
"=" {
    Token token(ASSIGN, cur);
    tokens.push_back(token);
}
<<EOF>> {
    eof = true;
    Token token(EOF_TOKEN, cur);
    tokens.push_back(token);
}
" "
\n {
    pos.line++;
    pos.column = 1;
}
. { std::cout << "failed to parse program"; throw "syntax error"; }
%%
int main() {
    while (!eof) {
        yylex();
    }
}
