
#include "builtin_fns.h"
#include <limits.h>

#define NOT_FOUND -1
#define ALPHABET_LEN (UCHAR_MAX + 1)

/* TODO +*/
int find(string *s, string *pattern);
/* TODO +*/
string *sort(string *s);

void create_suffix_table(int *suff_table, string *pattern);
void create_stop_symbols(int *stop_sbs, string *pattern);
int suffix_match(string *pattern, int offset, int suffix_len);
