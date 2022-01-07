

#ifndef html_builder_h
#define html_builder_h

#include <stdio.h>

char *strremove(char *str, const char *sub);

char *find_exts(char *filename);

int is_dir(const char *path);

int is_hide(char *name);

const char *table_row_create_string(char *href, char *value);

const char *table_row_create_int(char *href, long value);

char *build_html(char *subpath, char *root);

#endif /* html_builder_h */
