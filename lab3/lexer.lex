%option noyywrap
%{
#include <iostream>
#include <vector>
#include "lexer.h"

std::ostream &operator<<(std::ostream &out, const Position &position) {
    out << "(" << position.line << "," << position.column << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, const Fragment &fragment) {
    out << fragment.begin << "-" << fragment.end;
    return out;
}

std::ostream& operator<<(std::ostream &out, const Token &token) {
    out << token.fragment << " ";
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
        case DELIMITER:
            out << "DELIMITER";
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

Token yylex();

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
";" {
    return Token(DELIMITER, cur);
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
