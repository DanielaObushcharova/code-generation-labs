#include <iostream>
#include <sstream>
#include <ostream>
#include <vector>
#include "lexer.h"
#include "parser.h"

int global_id = 0;

SyntaxError::SyntaxError(const std::string &_msg, Token _token) {
    std::stringstream ss;
    ss << _msg << " at token " << _token;
    msg = ss.str();
}

const char * SyntaxError::what() const noexcept {
    return msg.c_str();
}

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
        case BB:
            out << "BB";
            break;
    }
    return out;
}

Node::Node(Rule _rule, Token *_token, const std::vector<Node*> &_children):
    rule(_rule), token(_token), children(_children), id(++global_id) {}

    void Node::print(bool header) {
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

void flattenS(Node *tree) {
    if (tree->rule == S && tree->children.size() == 2) {
        Node *sc = tree->children[1];
        flattenS(tree->children[1]);
        std::vector<Node*> children = {tree->children[0]->children[0]};
        for (Node *child : sc->children) {
            children.push_back(child);
        }
        tree->children = children;
    }
    for (Node *child : tree->children) {
        flattenS(child);
    }
}

void flattenRval(Node *tree) {
    std::vector<Node*> children;
    for (Node *child : tree->children) {
        if (child->rule == RVAL && child->children[1]->children.size() == 0) {
            children.push_back(child->children[0]);
        } else {
            flattenRval(child);
            children.push_back(child);
        }
    }
    tree->children = children;
}

void removeUnnecessaryTerms(Node *tree) {
    std::vector<Node*> children;
    for (Node *child : tree->children) {
        if (child->rule == TERM) {
            int type = child->token->type;
            if (
                    type != OPEN_BRACE 
                    && type != CLOSE_BRACE 
                    && type != IF 
                    && type != WHILE 
                    && type != RETURN 
                    && type != ELSE
                    && type != DELIMITER
                    && type != ASSIGN
               ) {
                children.push_back(child);
            }
        } else {
            removeUnnecessaryTerms(child);
            children.push_back(child);
        }
    }
    tree->children = children;
}

void insertBBVertices(Node *tree) {
    if (tree->rule == S) {
        std::vector<Node*> children;
        Node* curBB = new Node(BB, nullptr, {});
        for (Node *child : tree->children) {
            if (child->rule == ASSIGN_RULE) {
                curBB->children.push_back(child);
            } else {
                if (curBB->children.size() > 0) {
                    children.push_back(curBB);
                }
                children.push_back(child);
                curBB = new Node(BB, nullptr, {});
            }
        }
        if (curBB->children.size() > 0) {
            children.push_back(curBB);
        }
        tree->children = children;
    }
    for (Node *child : tree->children) {
        insertBBVertices(child);
    }
}

void convertToAST(Node *tree) {
    flattenS(tree);
    flattenRval(tree);
    removeUnnecessaryTerms(tree);
    insertBBVertices(tree);
}

Parser::Parser(const std::vector<Token> &_tokens):
    cur(0), tokens(_tokens) {}

    Token Parser::peek() {
        if (cur >= tokens.size()) {
            throw SyntaxError("syntax error, no tokens left to peek", tokens[tokens.size() - 1]);
        }
        return tokens[cur];
    }

void Parser::next() {
    ++cur;
}

Node *Parser::parse() {
    Node *s = parseS();
    if (peek().type != EOF_TOKEN) {
        throw SyntaxError("syntax error, not all tokens were parsed", peek());
    }
    convertToAST(s);
    return s;
}

Node *Parser::parseS() {
    Token t = peek();
    if (t.type != RETURN && t.type != IDENT && t.type != IF && t.type != WHILE) {
        return new Node(S, nullptr, {});
    }
    return new Node(S, nullptr, {parseExpr(), parseS()});
}

Node *Parser::parseExpr() {
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
            throw SyntaxError("syntax error, unknown expr", peek());
    }
    return new Node(EXPR, nullptr, {child});
}

Node *Parser::parseReturn() {
    Node *ret = parseToken(RETURN);
    Node *rval = parseRval();
    Node *delimiter = parseToken(DELIMITER);
    return new Node(RETURN_RULE, nullptr, {ret, rval, delimiter});
}

Node *Parser::parseAssign() {
    Node *ident = parseToken(IDENT);
    Node *assign = parseToken(ASSIGN);
    Node *rval = parseRval();
    Node *delimiter = parseToken(DELIMITER);
    return new Node(ASSIGN_RULE, nullptr, {ident, assign, rval, delimiter});
}

Node *Parser::parseIf() {
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

Node *Parser::parseWhile() {
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

Node *Parser::parseRval() {
    Node *sval = parseSval();
    Node *opval = parseOpval();
    return new Node(RVAL, nullptr,
            {
            sval,
            opval,
            });
}

Node *Parser::parseOpval() {
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

Node *Parser::parseSval() {
    Token t = peek();
    if (t.type == IDENT) {
        return parseToken(IDENT);
    } else if (t.type == NUMBER) {
        return parseToken(NUMBER);
    } else {
        throw SyntaxError("syntax error, unknown value", peek());
    }
}

Node *Parser::parseToken(TokenType type) {
    Token token = peek();
    if (token.type != type) {
        std::stringstream ss;
        ss << "syntax error, unexpected token type, expected " << type;
        throw SyntaxError(ss.str(), peek());
    }
    next();
    return new Node(TERM, new Token(token), {});
}
