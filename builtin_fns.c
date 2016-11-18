/** IFJ16
* Built-in functions.
**/

#include "builtin_fns.h"

/* TODO */
string *substr(string *s, int i, int n) {
    if (i < 0 || n < 0 || i > s->length || ((i + n) > s->length)) {
        throw_err(OTHER_ERROR, RET_VALUE);
    }
    string *to_return = new_string("");
    if (to_return != NULL) {
        while (n) {
            if (strAddChar(to_return, s->str[i++])) {
                del_strings(1, to_return);
            }
            n--;
        }
    }
    return to_return;
}

/* TODO */
int compare(string *s1, string *s2) {
    if ((s1 == NULL) || (s2 == NULL)) {
        throw_err(PR_ERROR, CALL_FUNC_ARG);
    }
    return strCmpString(s1, s2);
}

/* TODO */
int length(string *s) {
    if (s == NULL) {
        throw_err(PR_ERROR, CALL_FUNC_ARG);
    }
    return s->length;
}

/* TODO */
int read_int() {
    string buffer;
    strInit(&buffer);
    int result = 0;

    for (int curr_char = getchar(); curr_char != '\n' && curr_char != EOF; curr_char = getchar()) {
        if (strAddChar(&buffer, curr_char)) {
            strFree(&buffer);
            throw_err(ALLOC_ERROR, ALL_STRUCT);
        }
    }
    result = strtol((&buffer)->str, NULL, 0);
    if (result < 0) {
        strFree(&buffer);
        throw_err(INPUT_NUM_ERROR, RET_VALUE);
    }
    strFree(&buffer);
    return result;
}

/* TODO */
double read_double() {
    string buffer;
    strInit(&buffer);
    double result = 0.0;

    for (int curr_char = getchar(); curr_char != '\n' && curr_char != EOF; curr_char = getchar()) {
        if (strAddChar(&buffer, curr_char)) {
            strFree(&buffer);
            throw_err(ALLOC_ERROR, ALL_STRUCT);
        }
    }
    result = strtod((&buffer)->str, NULL);
    if (errno == ERANGE) {
        throw_err(INPUT_NUM_ERROR, RET_VALUE);
    }
    strFree(&buffer);
    return result;
}

/* TODO */
string *read_string() {
    string *to_return = new_string("");
    if (to_return != NULL) {
        for (int curr_char = getchar(); curr_char != '\n' && curr_char != EOF; curr_char = getchar()) {
            if (strAddChar(to_return, curr_char)) {
                del_strings(1, to_return);
                throw_err(ALLOC_ERROR, ALL_STRUCT);
            }
        }
    }
    else {
        throw_err(ALLOC_ERROR, ALL_STRUCT);
    }
    return to_return;
}
