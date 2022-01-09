#include <string.h>
#include "../../includes/parser.h"

const char *argp_program_version = "1.0.0";
const char *argp_program_bug_adress = "victor.gomes@icomp.ufam.edu.br";
static char doc[] = "Script used to extract game data.";
static char args_doc[] = "[absolute_path/to/pkg.idx]";
static struct argp_option options[] = {
    {"all", 'a', 0, 0, "Extract Everything"},
    {"icon", 'i', 0, 0, "Extract the game item, skill and UI icons"},
    {"ride", 'r', 0, 0, "Extract the game rides (animation, model and textures)"},
    {"data", 'd', 0, 0, "Extract the game .ini files with the item and monster descriptions"},
    {"output_path", 'o', "PATH", 0, "Choose an output path in which the files should be placed."},
    {0}};

static struct arguments arguments = {ALL_MODE, "\0"};
static struct argp argp = {options, parse_opt, args_doc, doc, 0, 0, 0};

error_t parse_opt(int key, char *arg, struct argp_state *state)
{

    struct arguments *args = state->input;

    switch (key)
    {

        case 'a': args->mode = ALL_MODE; break;
        case 'i': args->mode = ICONS_MODE; break;
        case 'r': args->mode = RIDE_MODE; break;
        case 'd': args->mode = DATA_MODE; break;
        case 'o': args->output_path = arg; break;
        case ARGP_KEY_ARG:
            if (state->arg_num >= 1)
                argp_error(state, "To many arguments");
            else
                args->pkg_path = arg;

            break;
        case ARGP_KEY_END:
            if (state->arg_num < 1)
                argp_error(state, "You need to provide the path to pkg.idx");

            break;
        default: return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

error_t parse(int argc, char **argv)
{

    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    return arguments.mode;
}

char *get_output_path()
{

    return arguments.output_path;
}

char *get_pkg_path()
{
    return arguments.pkg_path;
}