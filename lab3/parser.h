#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include <vector>
#include <string>
#include <memory>

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
    std::shared_ptr<Token> token;
    std::vector<std::shared_ptr<Node>> children;

    Node(Rule _rule, std::shared_ptr<Token> _token, const std::vector<std::shared_ptr<Node>> &_children);

    void print(bool header = true);
};

class Parser {
    private:
        int cur;
        std::vector<Token> tokens;
        Token peek(); 
        void next();
        std::shared_ptr<Node> parseS();
        std::shared_ptr<Node> parseExpr();
        std::shared_ptr<Node> parseReturn();
        std::shared_ptr<Node> parseAssign();
        std::shared_ptr<Node> parseIf();
        std::shared_ptr<Node> parseWhile();
        std::shared_ptr<Node> parseRval();
        std::shared_ptr<Node> parseOpval();
        std::shared_ptr<Node> parseSval();
        std::shared_ptr<Node> parseToken(TokenType type);

    public:
        Parser(const std::vector<Token> &_tokens);
        std::shared_ptr<Node> parse();
};

#endif
