#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "str.h"
#include "test_inst.h"
#include "test_scanner.h"
#include "test_table_remake.h"
#include "test_error.h"
#include "frame.h"
#include "test_interpret_remake.h"
#include "in_built.h"

#define SWAP(A, B) {char tmp = A; A = B; B = tmp;}
#define NOT_FOUND -1
#define ALPHABET_LEN (UCHAR_MAX + 1)
#ifndef max
#define max(a,b) ((a > b) ? a : b)
#endif // max

void bin_to_hex(char *str)
{
    printf("\n%s\n", "Hexa");
    while (*str) {
        printf("%02x ", *str);
        str++;
    }
    printf("\nEnd\n");
}

void activate_ride_symbol(char *string)
{
    int count = 0;
    while (*string)
    {
        if (string[0] == '\\' && string[1] == '\"')
        {
            count = 1;
            string[0] = '\"';
        }
        else if (string[0] == '\\' && string[1] == 'n')
        {
            count = 1;
            string[0] = 0xa;
        }
        else if (string[0] == '\\' && string[1] == 't')
        {
            count = 1;
            string[0] = 0x9;
        }
        else if (string[0] == '\\' && string[1] == '\\')
            count = 1;
        else if (string[0] == '\\' && isdigit(string[1]) &&
                isdigit(string[2]) && isdigit(string[3]))
        {
            char escape[3];
            for (int i = 0; i < 3; i++){

                escape[i] = string[1+i];
            }
            escape[3] = 0;
            unsigned oct;
            sscanf(escape, "%o", &oct);
            string[0] = oct;
            count = 3;
        }

        // posouvame symboly
        for (int j = 0; j < count; j++)
        {
            for (int i = 1; string[i]; i++)
            {
                string[i] = string[i + 1];
            }
        }
        count = 0;
        string++;
    }
}

void print(tData *printed)
{
    // potrebujeme novou polozku pro retezec k tisknuti
    string *str = convert_to_string(printed);
    if (printed->can_free)
    {
        if (printed->type == STRING)
        {
            strFree(&printed->value.str);
        }
        free(printed);
    }
    // vymenime urcite posloupnosti symbolu za jejich ridici analogy
    activate_ride_symbol(str->str);
    // vytiskneme
    fprintf(stdout, "%s", str->str);
    // uvolnime retezec
    strFree(str);
    free(str);
    //bin_to_hex(str);
}

void read_int(tExprStack *st)
{
    tData *new;
    if ((new = malloc(sizeof(tData))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    new->can_free = true;
    new->type = INT;
    if ((scanf("%d", &new->value.integer)) <= 0)
        throw_err(READ_ERROR, 0, 0);
    stack_expr_push(st, new);
}

void read_double(tExprStack *st)
{
    tData *new;
    if ((new = malloc(sizeof(tData))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    new->can_free = true;
    new->type = DOUBLE;
    if ((scanf("%lf", &new->value.real)) <= 0)
        throw_err(READ_ERROR, 0, 0);
    stack_expr_push(st, new);
}

void read_string(tExprStack *st)
{
    tData *new;
    if ((new = malloc(sizeof(tData))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    new->can_free = true;
    new->type = STRING;

    char buff[300];
    char *str;
    strInit(&new->value.str);
    str = fgets(buff, sizeof(buff), stdin);
    str[strlen(str)-1] = '\0';
    strWriteStr(&new->value.str, str);

    stack_expr_push(st, new);
}

void length(tExprStack *st)
{
    tData *str = stack_expr_pop(st);
    if (str->type != STRING)
        throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG, "ifj16.length");
    tData *new;
    if ((new = malloc(sizeof(tData))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    new->can_free = true;
    new->type = INT;

    string *str_ch = convert_to_string(str);
    activate_ride_symbol(str_ch->str);
    new->value.integer = strlen(str_ch->str);

    strFree(str_ch);
    free(str_ch);

    if (str->can_free)
        strFree(&str->value.str);
    stack_expr_push(st, new);
}

void sub_str(tExprStack *st)
{
    tData *length = stack_expr_pop(st);
    tData *beg = stack_expr_pop(st);
    tData *str = stack_expr_pop(st);

    if (length->type != INT ||
        beg->type != INT ||
        str->type != STRING)
        throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG, "ifj16.substr");
    int len = length->value.integer;
    int indx = beg->value.integer;
    if (indx > str->value.str.length)
        throw_err(OTHER_ERROR, CALL_FUNC_ARG, "ifj16.substr");

    tData *new;
    if ((new = malloc(sizeof(tData))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    strInit(&new->value.str);
    new->can_free = true;
    new->type = STRING;

    if (length->can_free)
        strFree(&length->value.str);
    if (beg->can_free)
        strFree(&beg->value.str);
    if (str->can_free)
        strFree(&str->value.str);

    while (len != 0 && indx < str->value.str.length)
    {
        strAddChar(&new->value.str, str->value.str.str[indx++]);
        len--;
    }
    stack_expr_push(st, new);
}

void compare(tExprStack *st)
{
    tData *s2 = stack_expr_pop(st);
    tData *s1 = stack_expr_pop(st);

    if (s1->type != STRING ||
        s2->type != STRING)
        throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG, "ifj16.compare");

    tData *new;
    if ((new = malloc(sizeof(tData))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    new->can_free = true;
    new->type = INT;

    int comp = strcmp(s1->value.str.str, s2->value.str.str);
    if (comp < 0)
        new->value.integer = -1;
    else if (comp > 0)
        new->value.integer = 1;
    else
        new->value.integer = 0;

    if (s1->can_free)
        strFree(&s1->value.str);
    if (s2->can_free)
        strFree(&s2->value.str);

    stack_expr_push(st, new);
}

char* sort (char *str)
{
    int length = strlen(str);
    for (int step = length/2; step > 0; step /= 2)
    {
		for (int j = step; j < length; j++)
        {
			for (int i = j - step; i >= 0 && str[i] > str[i + step]; i -= step)
				SWAP(str[i], str[i + step]);
		}
	}
    return str;
}

/**
* suffix_match
* TODO
* @param pattern    ->
* @param offset     ->
* @param suffix_len ->
* @return           ->
**/
int suffix_match(char *pattern, int offset, int suffix_len) {
    if (offset > suffix_len) {
        return pattern[offset - suffix_len - 1]
                != pattern[strlen(pattern) - suffix_len - 1]
                && memcmp(pattern + strlen(pattern) - suffix_len,
                        pattern + offset - suffix_len, suffix_len)
                == 0;
    }
    else {
        return memcmp(pattern + strlen(pattern) - offset,
                    pattern, offset) == 0;
    }
}
/**
* create_suffix_table
* TODO
* @param suff_table     ->
* @param pattern        ->
* @return               -> returns nothing
**/
void create_suffix_table(int *suff_table, char *pattern) {
    int patt_len = strlen(pattern);
    for (int i = 0; i < patt_len; i++) {
        int offset = patt_len;
        while (offset && !suffix_match(pattern, offset, i)) {
            --offset;
        }
        suff_table[patt_len - i - 1] = patt_len - offset;
    }
}
/**
* create_stop_symbols
* TODO
* @param stop_sbs   ->
* @param pattern    ->
* @return           -> returns nothing
**/
void create_stop_symbols(int *stop_sbs, char *pattern) {
    for (int i = 0; i < ALPHABET_LEN; i++) {
        stop_sbs[i] = strlen(pattern);
    }
    for (int i = 0; (unsigned)i < strlen(pattern) - 1; i++) {
        stop_sbs[(unsigned char)pattern[i]] = i;
    }
}
/**
* find
* TODO
* @param s          ->
* @param pattern    ->
* @return           ->
**/
int find(char *s, char *pattern)
{
	int s_len = strlen(s);
	int pat_len = strlen(pattern);
	int stop_symbols[ALPHABET_LEN];

	if (pat_len == 0)
		return 0;
	if (pat_len > s_len || s_len <= 0 || pat_len < 0 || !s || !pattern)
		return NOT_FOUND;

	int *suffix_table = malloc(sizeof(int)*pat_len);
	create_suffix_table(suffix_table, pattern);
	create_stop_symbols(stop_symbols, pattern);

	for (int s_pos = 0; s_pos <= s_len - pat_len; ) {
		int p_pos = pat_len - 1;

		while (pattern[p_pos] == s[p_pos + s_pos]) {
			if (p_pos == 0) {
				free(suffix_table);
				return s_pos;
			}
			--p_pos;
		}
		s_pos += max(suffix_table[p_pos],
			p_pos - stop_symbols[(unsigned char)s[p_pos + s_pos]]);
	}
	free(suffix_table);
	return NOT_FOUND;
}
