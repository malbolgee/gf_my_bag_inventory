#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../includes/pkg.h"
#include "../includes/utils.h"

void alloc_check(const void *mem, const char *message)
{

    if (mem == NULL)
        perror(message), exit(EXIT_FAILURE);
}

void open_file_check(const void *mem)
{

    alloc_check(mem, "Couldn't open the file.");
}

void ensure(bool expression, char *message)
{

    if (expression == false)
        fprintf(stderr, "%s\n", message), exit(EXIT_FAILURE);
}

void replace_char(char *string, char oc, char sc)
{

    char *ch = NULL;
    while ((ch = strchr(string, oc)))
        *ch = sc;
}

int compare(const void *a, const void *b)
{

    if (((package_t *)a)->pkg_num == ((package_t *)b)->pkg_num)
        if (((package_t *)a)->package_offset == ((package_t *)b)->package_offset)
            return 0;
        else if (((package_t *)a)->package_offset < ((package_t *)b)->package_offset)
            return -1;
        else
            return 1;
    else if (((package_t *)a)->pkg_num < ((package_t *)b)->pkg_num)
        return -1;
    else
        return 1;
}