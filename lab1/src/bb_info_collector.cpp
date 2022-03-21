#include <iostream>

#include "gcc-plugin.h"
#include "tree.h"
#include "gimple.h"
#include "bb_info_collector.h"

void bb_info_collector::add_string(std::string info) {
    this->s.append(info);
}

void bb_info_collector::add_op(tree_code op) {
    switch (op) {
        case LE_EXPR:
            this->add_string("<=");
            break;
        case LT_EXPR:
            this->add_string("<");
            break;
        case GE_EXPR:
            this->add_string(">=");
            break;
        case GT_EXPR:
            this->add_string(">");
            break;
        case EQ_EXPR:
            this->add_string("==");
            break;
        case PLUS_EXPR:
            this->add_string("+");
            break;
        case MINUS_EXPR:
            this->add_string("-");
            break;
        case MULT_EXPR:
            this->add_string("*");
            break;
        case FLOOR_DIV_EXPR:
        case EXACT_DIV_EXPR:
        case CEIL_DIV_EXPR:
        case ROUND_DIV_EXPR:
            this->add_string("/");
            break;
        case FLOOR_MOD_EXPR:
        case TRUNC_MOD_EXPR:
        case CEIL_MOD_EXPR:
        case ROUND_MOD_EXPR:
            this->add_string("%");
            break;
        default:
            this->add_string("<");
            this->add_string(std::to_string(op));
            this->add_string(">");
            break;
    }
}

void bb_info_collector::add_tree(tree t) {
    tree_code code = TREE_CODE(t);
    switch (code) {
        case CONSTRUCTOR:
            this->add_string("constructor");
            break;
        case INTEGER_CST:
            this->add_string(std::to_string(TREE_INT_CST_LOW(t)));
            break;
        case VAR_DECL:
        case CONST_DECL:
        case PARM_DECL:
        case FUNCTION_DECL:
        case LABEL_DECL:
            this->add_string(IDENTIFIER_POINTER(DECL_NAME(t)));
            break;
        case FUNCTION_TYPE:
            this->add_string("FUNC");
            break;
        case ARRAY_TYPE:
            this->add_string("ARRAY");
            break;
        case INTEGER_TYPE:
            this->add_string("INT");
            break;
        case ADDR_EXPR:
            this->add_string("&");
            this->add_tree(TREE_TYPE(t));
            break;
        case POINTER_TYPE:
            this->add_string("*");
            this->add_tree(TREE_TYPE(t));
            break;
        case SSA_NAME: {
            gimple *stmt = SSA_NAME_DEF_STMT(t);
            if (gimple_code(stmt) == GIMPLE_PHI) {
                this->add_string("PHI(");
                int args = gimple_phi_num_args(stmt);
                for (int i = 0; i < args; ++i) {
                    this->add_tree(gimple_phi_arg(stmt, i)->def);
                    if (i != args - 1) {
                        this->add_string(",");
                    }
                }
                this->add_string(")");
            } else {
                tree ident = SSA_NAME_IDENTIFIER(t);
                this->add_string(ident ? IDENTIFIER_POINTER(ident) : "unnamed_var");
                this->add_string(std::to_string(SSA_NAME_VERSION(t)));
            }
            break;
        }
        default:
            this->add_string("(");
            this->add_string(std::to_string(code));
            this->add_string(")");
    }
}

void bb_info_collector::add_gimple_assign(gimple *stmt) {
    this->add_string("ASSIGN ");
    switch (gimple_num_ops(stmt)) {
        case 2:
            this->add_tree(gimple_assign_lhs(stmt));
            this->add_string(" = ");
            this->add_tree(gimple_assign_rhs1(stmt));
            break;
        case 3:
            this->add_tree(gimple_assign_lhs(stmt));
            this->add_string(" = ");
            this->add_tree(gimple_assign_rhs1(stmt));
            this->add_string(" ");
            this->add_op(gimple_assign_rhs_code(stmt));
            this->add_string(" ");
            this->add_tree(gimple_assign_rhs2(stmt));
            break;
    }
}

void bb_info_collector::add_gimple_call(gimple *stmt) {
    this->add_tree(gimple_call_fn(stmt));
    this->add_string("(");
    int args = gimple_call_num_args(stmt);
    for (int i = 0; i < args; ++i) {
        this->add_tree(gimple_call_arg(stmt, i));
        if (i != args - 1) {
            this->add_string(",");
        }
    }
    this->add_string(")");
}

void bb_info_collector::add_gimple_cond(gimple *stmt) {
    this->add_string("COND ");
    this->add_tree(gimple_cond_lhs(stmt));
    this->add_string(" ");
    this->add_op(gimple_cond_code(stmt));
    this->add_string(" ");
    this->add_tree(gimple_cond_rhs(stmt));
}

void bb_info_collector::add_statement(gimple *stmt) {
    switch (gimple_code(stmt)) {
        case GIMPLE_ASSIGN:
            this->add_gimple_assign(stmt);
            break;
        case GIMPLE_CALL:
            this->add_gimple_call(stmt);
            break;
        case GIMPLE_COND:
            this->add_gimple_cond(stmt);
            break;
        case GIMPLE_RETURN:
            this->add_string("RETURN");
            break;
        case GIMPLE_LABEL:
            this->add_string("LABEL");
            break;
        case GIMPLE_PHI:
            this->add_string("PHI");
            break;
        default:
            this->add_string("Unknown statement: " + std::to_string(gimple_code(stmt)));
    }
    this->add_string("\n");
}

