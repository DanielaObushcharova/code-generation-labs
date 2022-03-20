#include <iostream>

#include "gcc-plugin.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "gimple-iterator.h"

int plugin_is_GPL_compatible = 1;

static struct plugin_info gimple_print_info = {
    .version = "0.1",
    .help = "This plugin prints GIMPLE",
}; 

static struct pass_data gimple_print_pass_data = {
    .type = GIMPLE_PASS,
    .name = "gimple_print",
};

struct gimple_print_pass : gimple_opt_pass {
    gimple_print_pass(gcc::context *ctx): gimple_opt_pass(gimple_print_pass_data, ctx) {}
    virtual gimple_print_pass *clone() override { return this; }
    virtual unsigned int execute(function *func) override;
};

void print_tree(tree t) {
    tree_code code = TREE_CODE(t);
    switch (code) {
        case INTEGER_CST:
            std::cout << TREE_INT_CST_LOW(t);
            break;
        case VAR_DECL:
        case CONST_DECL:
        case PARM_DECL:
        case FUNCTION_DECL:
        case LABEL_DECL:
            std::cout << IDENTIFIER_POINTER(DECL_NAME(t));
            break;
        case FUNCTION_TYPE:
            std::cout << "FUNC";
            break;
        case ARRAY_TYPE:
            std::cout << "ARRAY";
            break;
        case INTEGER_TYPE:
            std::cout << "INT";
            break;
        case ADDR_EXPR:
            std::cout << "&";
            print_tree(TREE_TYPE(t));
            break;
        case POINTER_TYPE:
            std::cout << "*";
            print_tree(TREE_TYPE(t));
            break;
        case SSA_NAME: {
            gimple *stmt = SSA_NAME_DEF_STMT(t);
            if (gimple_code(stmt) == GIMPLE_PHI) {
                std::cout << "PHI(";
                int args = gimple_phi_num_args(stmt);
                for (int i = 0; i < args; ++i) {
                    print_tree(gimple_phi_arg(stmt, i)->def);
                    if (i != args - 1) {
                        std::cout << ",";
                    }
                }
                std::cout << ")";
            } else {
                tree ident = SSA_NAME_IDENTIFIER(t);
                std::cout << (ident ? IDENTIFIER_POINTER(ident) : "unnamed_var") << SSA_NAME_VERSION(t);
            }
            break;
        }
        default:
            std::cout << "(" << code << ")";
    }
}

void print_op(tree_code op) {
    switch (op) {
        case EQ_EXPR:
            std::cout << "==";
            break;
        case PLUS_EXPR:
            std::cout << "+";
            break;
        case MINUS_EXPR:
            std::cout << "-";
            break;
        case MULT_EXPR:
            std::cout << "*";
            break;
        case EXACT_DIV_EXPR:
        case FLOOR_DIV_EXPR:
            std::cout << "/";
            break;
        case FLOAT_MOD_EXPR:
            std::cout << "%";
            break;
        default:
            std::cout << "<" << op << ">";
            break;
    }
}

void print_gimple_assign(gimple *stmt) {
    std::cout << "ASSIGN ";
    switch (gimple_num_ops(stmt)) {
        case 2:
            print_tree(gimple_assign_lhs(stmt));
            std::cout << " = ";
            print_tree(gimple_assign_rhs1(stmt));
            break;
        case 3:
            print_tree(gimple_assign_lhs(stmt));
            std::cout << " = ";
            print_tree(gimple_assign_rhs1(stmt));
            std::cout << " ";
            print_op(gimple_assign_rhs_code(stmt));
            std::cout << " ";
            print_tree(gimple_assign_rhs2(stmt));
            break;
    }
}

void print_gimple_call(gimple *stmt) {
    print_tree(gimple_call_fn(stmt));
    std::cout << "(";
    int args = gimple_call_num_args(stmt);
    for (int i = 0; i < args; ++i) {
        print_tree(gimple_call_arg(stmt, i));
        if (i != args - 1) {
            std::cout << ",";
        }
    }
    std::cout << ")";
}

void print_gimple_cond(gimple *stmt) {
    std::cout << "COND ";
    print_tree(gimple_cond_lhs(stmt));
    std::cout << " ";
    print_op(gimple_cond_code(stmt));
    std::cout << " ";
    print_tree(gimple_cond_rhs(stmt));
}

void print_gimple_stmt(gimple *stmt) {
    switch (gimple_code(stmt)) {
        case GIMPLE_ASSIGN:
            print_gimple_assign(stmt);
            break;
        case GIMPLE_CALL:
            print_gimple_call(stmt);
            break;
        case GIMPLE_COND:
            print_gimple_cond(stmt);
            break;
        case GIMPLE_RETURN:
            std::cout << "RETURN";
            break;
        case GIMPLE_LABEL:
            std::cout << "LABEL";
            break;
        case GIMPLE_PHI:
            std::cout << "PHI";
            break;
        default:
            std::cout << "Unknown statement: " << gimple_code(stmt);
    }
}

unsigned int gimple_print_pass::execute(function *func) {
    basic_block bb;
    FOR_ALL_BB_FN(bb, func) {
        gimple_stmt_iterator it;
        for (it = gsi_start_bb(bb); !gsi_end_p(it); gsi_next(&it)) {
            gimple *stmt = gsi_stmt(it);
            print_gimple_stmt(stmt);
            std::cout << std::endl;
        }
    }
    return 0;
}

static struct register_pass_info gimple_print_pass_info = {
    .pass = new gimple_print_pass(g),
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER,
};

int plugin_init(struct plugin_name_args *args, struct plugin_gcc_version *version)
{
    register_callback(args->base_name, PLUGIN_INFO, NULL, &gimple_print_info);
    register_callback(args->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &gimple_print_pass_info);
    return 0;
}
