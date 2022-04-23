#include "parser.h"

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
        tree->print();
    } catch(SyntaxError err) {
        std::cout << err.what() << std::endl;
    }
}
