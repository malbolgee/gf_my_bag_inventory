#ifndef __UTILS_H__
#define __UTILS_H__

void usage();
void ensure(bool expression, char *message);
void open_file_check(const void *mem);
void alloc_check(const void *mem, const char *message);
void replace_char(char *string, char oc, char sc);
int compare(const void *a, const void *b);

#endif