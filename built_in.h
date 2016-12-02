#ifndef _IN_BUILT_
#define _IN_BUILT_

void print(tData *printed);
void read_int(tExprStack *st);
void read_double(tExprStack *st);
void read_string(tExprStack *st);

void length(tExprStack *st);
void sub_str(tExprStack *st);
void compare(tExprStack *st);

char* sort (char *str);


/* TODO +*/
int find(char *s, char *pattern);

void create_suffix_table(int *suff_table, char *pattern);
void create_stop_symbols(int *stop_sbs, char *pattern);
int suffix_match(char *pattern, int offset, int suffix_len);

#endif
