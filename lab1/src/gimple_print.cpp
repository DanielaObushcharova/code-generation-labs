#include <iostream>

#include "gcc-plugin.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "gimple-iterator.h"

#include "bb_info_collector.h"

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

unsigned int gimple_print_pass::execute(function *func) {
    basic_block bb;
    FOR_ALL_BB_FN(bb, func) {
        gimple_stmt_iterator it;
        bb_info_collector info;
        for (it = gsi_start_bb(bb); !gsi_end_p(it); gsi_next(&it)) {
            gimple *stmt = gsi_stmt(it);
            info.add_statement(stmt);
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
