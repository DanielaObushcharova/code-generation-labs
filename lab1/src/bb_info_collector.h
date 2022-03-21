#ifndef BB_INFO_COLLECTOR_H
#define BB_INFO_COLLECTOR_H

#include <iostream>
#include "gcc-plugin.h"

class bb_info_collector {
private:
    std::string s;

    void add_gimple_assign(gimple *stmt);
    void add_gimple_call(gimple *stmt);
    void add_gimple_cond(gimple *stmt);
    void add_tree(tree t);
    void add_op(tree_code op);
    void add_string(std::string info);
public:
    bb_info_collector() {};
    bb_info_collector(gimple *stmt);

    void add_statement(gimple *stmt);
};

#endif
