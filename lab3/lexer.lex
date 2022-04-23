%{
#include <iostream>
#include <vector>

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

    std::ostream& operator<<(std::ostream &out, const Token &token) {
        switch (token.type) {
            case NUMBER:
                out << "NUMBER(" << token.numberAttr << ")";
                break;
            case IF:
                out << "IF";
                break;
            case ELSE:
                out << "ELSE";
                break;
            case OPEN_BRACE:
                out << "OPEN_BRACE";
                break;
            case CLOSE_BRACE:
                out << "CLOSE_BRACE";
                break;
            case WHILE:
                out << "WHILE";
                break;
            case IDENT:
                out << "IDENT(" << token.identAttr << ")";
                break;
            case OP:
                out << "OP(" << token.opAttr << ")";
                break;
            case ASSIGN:
                out << "ASSIGN";
                break;
            case RETURN:
                out << "RETURN";
                break;
            case EOF_TOKEN:
                out << "EOF";
                break;
        }
        return out;
    }

#define YY_DECL Token yylex()

extern "C" Token yylex();

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

#define YY_USER_ACTION { run_user_action(); }
%}
IDENT [a-zA-Z][a-zA-Z0-9]*
NUMBER -?(0|[1-9][0-9]*)
OP [*+-]
%%
"return" {
    return Token(RETURN, cur);
}
"if" {
    return Token(IF, cur);
}
"while" {
    return Token(WHILE, cur);
}
"else" {
    return Token(ELSE, cur);
}
{IDENT} {
    Token token(IDENT, cur);
    token.identAttr = yytext;
    return token;
}
{NUMBER} {
    Token token(NUMBER, cur);
    token.numberAttr = std::stoi(yytext);
    return token;
}
{OP} {
    Token token(OP, cur);
    token.opAttr = yytext[0];
    return token;
}
"{" {
    return Token(OPEN_BRACE, cur);
}
"}" {
    return Token(CLOSE_BRACE, cur);
}
"=" {
    return Token(ASSIGN, cur);
}
<<EOF>> {
    eof = true;
    return Token(EOF_TOKEN, cur);
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
        Token token = yylex();
        std::cout << token << std::endl;
    }
}
