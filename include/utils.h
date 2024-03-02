#ifndef __UTILS_H_
#define __UTILS_H_

#include <stdlib.h>

void die(int rc, char *msg);
void print_help(int rc);
void *xmalloc(size_t cnt);

#endif
