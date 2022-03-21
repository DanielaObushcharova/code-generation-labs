#ifndef BB_INFO_COLLECTOR_H
#define BB_INFO_COLLECTOR_H

#include <iostream>
#include <vector>

#include "gcc-plugin.h"

class bb_info_collector {
private:
    int id;
    std::string s;
    std::vector<int> adjacent;

    void add_gimple_assign(gimple *stmt);
    void add_gimple_call(gimple *stmt);
    void add_gimple_cond(gimple *stmt);
    void add_tree(tree t);
    void add_op(tree_code op);
    void add_string(std::string info);

public:
    bb_info_collector(int id);

    void add_statement(gimple *stmt);
    void add_adjacent(int id);
    void print_graphviz();
};

#endif
