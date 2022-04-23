#include <iostream>
#include <ostream>
#include <vector>
#include "lexer.h"

int global_id = 0;

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
};

std::ostream &operator<<(std::ostream &out, const Rule &rule) {
    switch (rule) {
    case S:
        out << "S";
        break;
    case EXPR:
        out << "EXPR";
        break;
    case RETURN_RULE:
        out << "RETURN";
        break;
    case ASSIGN_RULE:
        out << "ASSIGN";
        break;
    case IF_RULE:
        out << "IF";
        break;
    case WHILE_RULE:
        out << "WHILE";
        break;
    case RVAL:
        out << "RVAL";
        break;
    case OPVAL:
        out << "OPVAL";
        break;
    case SVAL:
        out << "SVAL";
        break;
    case TERM:
        out << "TERM";
        break;
    }
    return out;
}

struct Node {
    Rule rule;
    int id;
    Token *token;
    std::vector<Node*> children;

    Node(Rule _rule, Token *_token, const std::vector<Node*> &_children):
        rule(_rule), token(_token), children(_children), id(++global_id) {}

    void print(bool header = true) {
        if (header) {
            std::cout << "digraph G {" << std::endl;
        }
        if (token) {
            std::cout << id << "[label=\"" << *token << "\"]" << std::endl;
        } else {
            std::cout << id << "[label=\"" << rule << "\"]" << std::endl;
        }

        for (Node *child : children) {
            std::cout << id << "->" << child->id << std::endl;
            child->print(false);
        }
        if (header) {
            std::cout << "}" << std::endl;
        }
    }
};

class Parser {
private:
    int cur;
    std::vector<Token> tokens;

public:
    Parser(const std::vector<Token> &_tokens):
        cur(0), tokens(_tokens) {}

    Token peek() {
        if (cur >= tokens.size()) {
            throw "syntax error, no tokens left to peek";
        }
        return tokens[cur];
    }

    void next() {
        ++cur;
    }

    Node *parse() {
        Node *s = parseS();
        if (peek().type != EOF_TOKEN) {
            throw "syntax error, not all tokens were parsed";
        }
        return s;
    }

    Node *parseS() {
        Token t = peek();
        if (t.type != RETURN && t.type != IDENT && t.type != IF && t.type != WHILE) {
            return new Node(S, nullptr, {});
        }
        return new Node(S, nullptr, {parseExpr(), parseS()});
    }

    Node *parseExpr() {
        Node *child;
        switch (peek().type) {
        case RETURN:
            child = parseReturn();
            break;
        case IDENT:
            child = parseAssign();
            break;
        case IF:
            child = parseIf();
            break;
        case WHILE:
            child = parseWhile();
            break;
        default:
            throw "syntax error, unknown expr";
        }
        return new Node(EXPR, nullptr, {child});
    }

    Node *parseReturn() {
        Node *ret = parseToken(RETURN);
        Node *rval = parseRval();
        Node *delimiter = parseToken(DELIMITER);
        return new Node(RETURN_RULE, nullptr, {ret, rval, delimiter});
    }

    Node *parseAssign() {
        Node *ident = parseToken(IDENT);
        Node *assign = parseToken(ASSIGN);
        Node *rval = parseRval();
        Node *delimiter = parseToken(DELIMITER);
        return new Node(ASSIGN_RULE, nullptr, {ident, assign, rval, delimiter});
    }

    Node *parseIf() {
        Node *ifkw = parseToken(IF);
        Node *rval = parseRval();
        Node *opbrace1 = parseToken(OPEN_BRACE);
        Node *sif = parseS();
        Node *cbrace1 = parseToken(CLOSE_BRACE);
        Node *elsekw = parseToken(ELSE);
        Node *opbrace2 = parseToken(OPEN_BRACE);
        Node *selse = parseS();
        Node *cbrace2 = parseToken(CLOSE_BRACE);
        return new Node(IF_RULE, nullptr, 
                {
                   ifkw,
                   rval,
                   opbrace1,
                   sif,
                   cbrace1,
                   elsekw,
                   opbrace2,
                   selse,
                   cbrace2,
                });
    }

    Node *parseWhile() {
        Node *whilekw = parseToken(WHILE);
        Node *rval = parseRval();
        Node *opbrace = parseToken(OPEN_BRACE);
        Node *s = parseS();
        Node *cbrace = parseToken(CLOSE_BRACE);
        return new Node(WHILE_RULE, nullptr,
                {
                    whilekw,
                    rval,
                    opbrace,
                    s,
                    cbrace,
                });
    }

    Node *parseRval() {
        Node *sval = parseSval();
        Node *opval = parseOpval();
        return new Node(RVAL, nullptr,
                {
                sval,
                opval,
                });
    }

    Node *parseOpval() {
        if (peek().type != OP) {
            return new Node(OPVAL, nullptr, {});
        }
        Node *op = parseToken(OP);
        Node *sval = parseSval();
        return new Node(OPVAL, nullptr,
            {
                op,
                sval,
            });
    }

    Node *parseSval() {
        Token t = peek();
        if (t.type == IDENT) {
            return parseToken(IDENT);
        } else if (t.type == NUMBER) {
            return parseToken(NUMBER);
        } else {
            throw "syntax error, unknown value";
        }
    }

    Node *parseToken(TokenType type) {
        Token token = peek();
        if (token.type != type) {
            throw "syntax error, unexpected token type";
        }
        next();
        return new Node(TERM, new Token(token), {});
    }
};

int main() {
    std::vector<Token> tokens;
    while (true) {
        Token token = yylex();
        tokens.push_back(token);
        if (token.type == EOF_TOKEN) {
            break;
        }
    }
    Parser parser(tokens);
    Node *tree;
    try {
        tree = parser.parse();
    } catch(char const *s) {
        std::cout << s << std::endl;
        std::cout << "at token: " << parser.peek() << std::endl;
    }
    tree->print();
}
