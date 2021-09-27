#ifndef __PARSER_H__
#define __PARSER_H__

#include <argp.h>

struct arguments
{
    enum
    {
        ALL_MODE,
        ICONS_MODE,
        RIDE_MODE,
        DATA_MODE
    } mode;

    char *output_path;
    char *pkg_path;
};

error_t parse(int argc, char **argv);
error_t parse_opt(int key, char *arg, struct argp_state *state);
char *get_output_path();
char *get_pkg_path();

#endif