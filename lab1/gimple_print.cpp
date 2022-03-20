#include <gcc-plugin.h>

int plugin_is_GPL_compatible = 1;

static struct plugin_info gimple_print_info = {
    .version = "0.1",
    .help = "This plugin prints GIMPLE",
}; 

int plugin_init(struct plugin_name_args *args, struct plugin_gcc_version *version)
{
    register_callback(args->base_name, PLUGIN_INFO, NULL, &gimple_print_info);
    return 0;
}
