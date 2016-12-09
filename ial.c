
#include "ial.h"

#ifndef max
#define max(a,b) ((a > b) ? a : b)
#endif // max


/**
* suffix_match
* TODO
* @param pattern    ->
* @param offset     ->
* @param suffix_len ->
* @return           ->
**/
int suffix_match(string *pattern, int offset, int suffix_len) {
    if (offset > suffix_len) {
        return pattern->str[offset - suffix_len - 1]
                != pattern->str[pattern->length - suffix_len - 1]
                && memcmp(pattern->str + pattern->length - suffix_len,
                        pattern->str + offset - suffix_len, suffix_len)
                == 0;
    }
    else {
        return memcmp(pattern->str + pattern->length - offset,
                    pattern->str, offset) == 0;
    }
}
/**
* create_suffix_table
* TODO
* @param suff_table     ->
* @param pattern        ->
* @return               -> returns nothing
**/
void create_suffix_table(int *suff_table, string *pattern) {
    int patt_len = pattern->length;
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
void create_stop_symbols(int *stop_sbs, string *pattern) {
    for (int i = 0; i < ALPHABET_LEN; i++) {
        stop_sbs[i] = pattern->length;
    }
    for (int i = 0; i < pattern->length - 1; i++) {
        stop_sbs[(unsigned char)pattern->str[i]] = i;
    }
}
/**
* find
* TODO
* @param s          ->
* @param pattern    ->
* @return           ->
**/
int find(string *s, string *pattern) {

	int s_len = s->length;
	int pat_len = pattern->length;
	int stop_symbols[ALPHABET_LEN];

	if (pat_len == 0) {
		return 0;
	}
	if (pat_len > s_len || s_len <= 0 || pat_len < 0 || !s || !pattern) {
		return NOT_FOUND;
	}

	int *suffix_table = malloc(sizeof(int)*pat_len);
	create_suffix_table(suffix_table, pattern);
	create_stop_symbols(stop_symbols, pattern);

	for (int s_pos = 0; s_pos <= s_len - pat_len; ) {
		int p_pos = pat_len - 1;

		while (pattern->str[p_pos] == s->str[p_pos + s_pos]) {
			if (p_pos == 0) {
				free(suffix_table);
				return s_pos;
			}
			--p_pos;
		}
		s_pos += max(suffix_table[p_pos],
			p_pos - stop_symbols[(unsigned char)s->str[p_pos + s_pos]]);
	}
	free(suffix_table);
	return NOT_FOUND;
}


/* TODO */
string *sort(string *s) {
    if (s->str == NULL) {
        return NULL;
    }
    string *sorted = new_string(s->str);
    if (sorted == NULL) {
        return NULL;
    }

    int N = sorted->length;
    int step, i, j;
    char curr_char;

    for (step = N/2; step > 0; step /= 2) {
        for (i = step; i < N; i++) {
            curr_char = sorted->str[i];

            for (j = i; j >= step; j -= step) {
                if (curr_char < sorted->str[j-step]) {
                    sorted->str[j] = sorted->str[j-step];
                }
                else {
                    break;
                }
            }
            sorted->str[j] = curr_char;
        }
    }
    return sorted;
}
