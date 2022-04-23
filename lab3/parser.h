#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <vector>
#include <string>

enum Rule {
    S,
    EXPR,
    RETURN_RULE,
    ASSIGN_RULE,
    IF_RULE,
    WHILE_RULE,
    RVAL,
    OPVAL,
    SVAL,
    TERM,
    BB,
};

std::ostream &operator<<(std::ostream &out, const Rule &rule);

class SyntaxError : std::exception {
    private:
        std::string msg;

    public:
        SyntaxError(const std::string &_msg, Token _token);

        const char * what() const noexcept override;
};

struct Node {
    Rule rule;
    int id;
    Token *token;
    std::vector<Node*> children;

    Node(Rule _rule, Token *_token, const std::vector<Node*> &_children);

    void print(bool header = true);
};

class Parser {
    private:
        int cur;
        std::vector<Token> tokens;
        Token peek(); 
        void next();
        Node *parseS();
        Node *parseExpr();
        Node *parseReturn();
        Node *parseAssign();
        Node *parseIf();
        Node *parseWhile();
        Node *parseRval();
        Node *parseOpval();
        Node *parseSval();
        Node *parseToken(TokenType type);

    public:
        Parser(const std::vector<Token> &_tokens);
        Node *parse();
};

#endif
