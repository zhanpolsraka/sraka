/** IFJ16
* Built-in functions.
**/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <errno.h>
#include "str.h"
#include "test_error.h"



/* TODO +*/
int read_int();
/* TODO +*/
double read_double();
/* TODO +*/
string *read_string();
/* TODO +*/
void print(string *to_print);
/* TODO +*/
int length(string *s);
/* TODO +*/
string *substr(string *s, int i, int n);
/* TODO +*/
int compare(string *s1, string *s2);
/* TODO +*/
string *dtos(double to_conv);
/* TODO +*/
string *itos(int to_conv);
