#include "parser.h"
#include "lexer.h"
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>

int global_id = 0;

SyntaxError::SyntaxError(const std::string &_msg, Token _token) {
  std::stringstream ss;
  ss << _msg << " at token " << _token;
  msg = ss.str();
}

const char *SyntaxError::what() const noexcept { return msg.c_str(); }

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

Node::Node(Rule _rule, std::shared_ptr<Token> _token,
           const std::vector<std::shared_ptr<Node>> &_children)
    : rule(_rule), token(_token), children(_children), id(++global_id) {}

void Node::print(bool header) {
  if (header) {
    std::cout << "digraph G {" << std::endl;
  }
  if (token) {
    std::cout << id << "[label=\"" << *token << "\"]" << std::endl;
  } else {
    std::cout << id << "[label=\"" << rule << "\"]" << std::endl;
  }

  for (std::shared_ptr<Node> child : children) {
    std::cout << id << "->" << child->id << std::endl;
    child->print(false);
  }
  if (header) {
    std::cout << "}" << std::endl;
  }
}

void flattenS(std::shared_ptr<Node> tree) {
  if (tree->rule == S && tree->children.size() == 2 && tree->children[0]->rule == EXPR) {
    std::shared_ptr<Node> sc = tree->children[1];
    flattenS(tree->children[1]);
    std::vector<std::shared_ptr<Node>> children = {
        tree->children[0]->children[0]};
    for (std::shared_ptr<Node> child : sc->children) {
      children.push_back(child);
    }
    tree->children = children;
  }
  for (std::shared_ptr<Node> child : tree->children) {
    flattenS(child);
  }
}

void flattenRval(std::shared_ptr<Node> tree) {
  std::vector<std::shared_ptr<Node>> children;
  for (std::shared_ptr<Node> child : tree->children) {
    if (child->rule == RVAL && child->children[1]->children.size() == 0) {
      children.push_back(child->children[0]);
    } else {
      flattenRval(child);
      children.push_back(child);
    }
  }
  tree->children = children;
}

void removeUnnecessaryTerms(std::shared_ptr<Node> tree) {
  std::vector<std::shared_ptr<Node>> children;
  for (std::shared_ptr<Node> child : tree->children) {
    if (child->rule == TERM) {
      int type = child->token->type;
      if (type != OPEN_BRACE && type != CLOSE_BRACE && type != IF &&
          type != WHILE && type != RETURN && type != ELSE &&
          type != DELIMITER && type != ASSIGN) {
        children.push_back(child);
      }
    } else {
      removeUnnecessaryTerms(child);
      children.push_back(child);
    }
  }
  tree->children = children;
}

void insertBBVertices(std::shared_ptr<Node> tree) {
  if (tree->rule == S) {
    std::vector<std::shared_ptr<Node>> children;
    std::shared_ptr<Node> curBB = std::make_shared<Node>(Node(BB, nullptr, {}));
    for (std::shared_ptr<Node> child : tree->children) {
      if (child->rule == ASSIGN_RULE || child->rule == RETURN_RULE) {
        curBB->children.push_back(child);
      }
      if (child->rule != ASSIGN_RULE) {
        if (curBB->children.size() > 0) {
          children.push_back(curBB);
        }
        if (child->rule != RETURN_RULE) {
          children.push_back(child);
        }
        curBB = std::make_shared<Node>(Node(BB, nullptr, {}));
      }
    }
    if (curBB->children.size() > 0) {
      children.push_back(curBB);
    }
    tree->children = children;
  }
  for (std::shared_ptr<Node> child : tree->children) {
    insertBBVertices(child);
  }
}

void convertToAST(std::shared_ptr<Node> tree) {
  flattenS(tree);
  flattenRval(tree);
  removeUnnecessaryTerms(tree);
  insertBBVertices(tree);
}

Parser::Parser(const std::vector<Token> &_tokens) : cur(0), tokens(_tokens) {}

Token Parser::peek() {
  if (cur >= tokens.size()) {
    throw SyntaxError("syntax error, no tokens left to peek",
                      tokens[tokens.size() - 1]);
  }
  return tokens[cur];
}

void Parser::next() { ++cur; }

std::shared_ptr<Node> Parser::parse() {
  std::shared_ptr<Node> s = parseS();
  if (peek().type != EOF_TOKEN) {
    throw SyntaxError("syntax error, not all tokens were parsed", peek());
  }
  convertToAST(s);
  return s;
}

std::shared_ptr<Node> Parser::parseS() {
  Token t = peek();
  if (t.type != RETURN && t.type != IDENT && t.type != IF && t.type != WHILE) {
    return std::make_shared<Node>(Node(S, nullptr, {}));
  }
  return std::make_shared<Node>(Node(S, nullptr, {parseExpr(), parseS()}));
}

std::shared_ptr<Node> Parser::parseExpr() {
  std::shared_ptr<Node> child;
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
  return std::make_shared<Node>(Node(EXPR, nullptr, {child}));
}

std::shared_ptr<Node> Parser::parseReturn() {
  std::shared_ptr<Node> ret = parseToken(RETURN);
  std::shared_ptr<Node> rval = parseRval();
  std::shared_ptr<Node> delimiter = parseToken(DELIMITER);
  return std::make_shared<Node>(
      Node(RETURN_RULE, nullptr, {ret, rval, delimiter}));
}

std::shared_ptr<Node> Parser::parseAssign() {
  std::shared_ptr<Node> ident = parseToken(IDENT);
  std::shared_ptr<Node> assign = parseToken(ASSIGN);
  std::shared_ptr<Node> rval = parseRval();
  std::shared_ptr<Node> delimiter = parseToken(DELIMITER);
  return std::make_shared<Node>(
      Node(ASSIGN_RULE, nullptr, {ident, assign, rval, delimiter}));
}

std::shared_ptr<Node> Parser::parseIf() {
  std::shared_ptr<Node> ifkw = parseToken(IF);
  std::shared_ptr<Node> rval = parseRval();
  std::shared_ptr<Node> opbrace1 = parseToken(OPEN_BRACE);
  std::shared_ptr<Node> sif = parseS();
  std::shared_ptr<Node> cbrace1 = parseToken(CLOSE_BRACE);
  std::shared_ptr<Node> elsekw = parseToken(ELSE);
  std::shared_ptr<Node> opbrace2 = parseToken(OPEN_BRACE);
  std::shared_ptr<Node> selse = parseS();
  std::shared_ptr<Node> cbrace2 = parseToken(CLOSE_BRACE);
  return std::make_shared<Node>(Node(IF_RULE, nullptr,
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
                                     }));
}

std::shared_ptr<Node> Parser::parseWhile() {
  std::shared_ptr<Node> whilekw = parseToken(WHILE);
  std::shared_ptr<Node> rval = parseRval();
  std::shared_ptr<Node> opbrace = parseToken(OPEN_BRACE);
  std::shared_ptr<Node> s = parseS();
  std::shared_ptr<Node> cbrace = parseToken(CLOSE_BRACE);
  return std::make_shared<Node>(Node(WHILE_RULE, nullptr,
                                     {
                                         whilekw,
                                         rval,
                                         opbrace,
                                         s,
                                         cbrace,
                                     }));
}

std::shared_ptr<Node> Parser::parseRval() {
  std::shared_ptr<Node> sval = parseSval();
  std::shared_ptr<Node> opval = parseOpval();
  return std::make_shared<Node>(Node(RVAL, nullptr,
                                     {
                                         sval,
                                         opval,
                                     }));
}

std::shared_ptr<Node> Parser::parseOpval() {
  if (peek().type != OP) {
    return std::make_shared<Node>(Node(OPVAL, nullptr, {}));
  }
  std::shared_ptr<Node> op = parseToken(OP);
  std::shared_ptr<Node> sval = parseSval();
  return std::make_shared<Node>(Node(OPVAL, nullptr,
                                     {
                                         op,
                                         sval,
                                     }));
}

std::shared_ptr<Node> Parser::parseSval() {
  Token t = peek();
  if (t.type == IDENT) {
    return parseToken(IDENT);
  } else if (t.type == NUMBER) {
    return parseToken(NUMBER);
  } else {
    throw SyntaxError("syntax error, unknown value", peek());
  }
}

std::shared_ptr<Node> Parser::parseToken(TokenType type) {
  Token token = peek();
  if (token.type != type) {
    std::stringstream ss;
    ss << "syntax error, unexpected token type, expected " << type;
    throw SyntaxError(ss.str(), peek());
  }
  next();
  return std::make_shared<Node>(
      Node(TERM, std::make_shared<Token>(Token(token)), {}));
}
