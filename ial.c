/* **************************************************************************/
/* Projekt:             Implementace interpretu jazyka IFJ16				*/
/* Predmet:             Formalni jazyky a prekladace (IFJ)					*/
/* Soubor:              ial.c                           		   			*/
/*																			*/
/* Autor login:      	Ermak Aleksei		xermak00						*/
/*                     	Khaitovich Anna		xkhait00						*/
/*						Nesmelova Antonina	xnesmel00						*/
/*						Fedorenko Oleh		xfedor07						*/
/*						Fedin Evgenii		xfedin00						*/
/* **************************************************************************/

/* Poznamka: Tento soubor nebude pri prekladu vyuzit */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#include "str.h"
#include "scanner.h"
#include "error.h"
#include "table.h"
#include "buffer.h"

#define ALPHABET_LEN (UCHAR_MAX + 1)
#define NOT_FOUND -1
#define max(a,b) ((a > b) ? a : b)
#define SWAP(A, B) {char tmp = A; A = B; B = tmp;}

// globalni ukazatel na strom
tTree *tree;
// pocet argumentu funkci
int numb_arg = 0;

/** Kopie
* Original se nachazi ve souboru table.c na radku 31.
* Vytvori, inicializuje a vlozi novy uzel do tabulky symbolu.
*/
void create_node(string *id, int type, int n_type, bool stat)
{
	// pokud pred operaci se zpracovavali argumenty, nastavime pocet argumentu na nulu
	if (numb_arg)
		numb_arg = 0;
	// nazev nesmi obsahovat tecku
	if (strchr(strGetStr(id), '.') != NULL)
		throw_err(SYN_ERROR, UNK_EXPR, id->str);

	// vytvorime novy uzel
	tNode *new;
	if ((new = malloc(sizeof(tNode))) == NULL)
		throw_err(INT_ERROR, ALL_STRUCT, 0);
	mark_mem(new);
	strInit(&new->key);

	// vytvorime klic pro uzel a inicializujeme ho
	strCopyString(&new->key, id);
	new->node_type = n_type;
    new->type = type;
	new->l_node = NULL;
	new->r_node = NULL;

    // pomocny uzel, ukazujici kam mame ulozit novy uzel
	tNode *begin = NULL;
    // na zaklade typu uzlu urcime kam mame ulozit novy uzel
	switch (new->node_type)
	{
		case VARIABLE:
            new->type = type;
			new->argument = 0;       // promenna neni argument funkci
            new->variables = NULL;	// nepouzijeme pro promenne
			new->functions = NULL;
            // pokud promenna je staticka, ulozime ji do aktivni tridy
			if (stat)
				begin = tree->active_class->variables;
			else
            // jinak ulozime ji do aktivni funkci
				begin = tree->active_func->variables;
		break;

		case FUNCTION:
            new->type = type;        // nastavime typ navratove hodnoty
			new->argument = 0;       // zatim funkce nema argumenty...
			new->variables = NULL;   // ...a promenne taky
			new->functions = NULL;
            // ulozime do stromu funkci aktivni tridy
			begin = tree->active_class->functions;
		break;

		case CLASS:
			new->type = 0;
			new->functions = NULL;
			new->variables = NULL;
			begin = tree->root;
		break;
	}
	// neexistuje korenovy uzel -> ulozime jako prvni tridu
	if (tree->root == NULL && new->node_type == CLASS)
	{
		tree->root = new;
		tree->active_class = new;
	}
	// ve tride neexistuji funkce -> ulozime jako prvni funkci
	else if (new->node_type == FUNCTION && tree->active_class->functions == NULL)
	{
		tree->active_class->functions = new;
		tree->active_func = new;
	}
	// ve tride neexistuji staticke promenne -> ulozime jako prvni st. promennou
	else if (new->node_type == VARIABLE && stat && tree->active_class->variables == NULL)
		tree->active_class->variables = new;
	// ve funkci neexistuji promenne -> ulozime jako prvni promennou
	else if (new->node_type == VARIABLE && tree->active_func != NULL &&
		tree->active_func->variables == NULL && !stat)
		tree->active_func->variables = new;
    // jinak uloz uzal tam kam ma
    else
		add_node(new, begin);
}

/** Kopie
* Original se nachazi ve souboru table.c na radku 131.
* Hledani urciteho uzlu podle nazvu.
*/
tNode *get_node(string *key, int n_type, tNode *begin)
{
	if (tree->root == NULL)
		return NULL;
	else if (begin == NULL)
    // zacneme vyhledavani od zacatku stromu (pro hledani trid)
		begin = tree->root;

	if (begin != NULL)
	{
		// porovname klice
		int cmp = strCmpString(key, &begin->key);
		// jdeme do leveho podstromu
		if (cmp < 0)
		{
			if (begin->l_node != NULL)
				return get_node(key, n_type, begin->l_node);
			else
				return NULL;
		}
		// jdeme do praveho podstromu
		else if (cmp > 0)
		{
			if (begin->r_node != NULL)
				return get_node(key, n_type, begin->r_node);
			else
				return NULL;
		}
		// nasli uzel
		else if (cmp == 0 && begin->node_type == n_type)
			return begin;
		else
			throw_err(INT_ERROR, 0, 0);
	}
	return NULL;
}

/** Kopie
* Original se nachazi ve souboru table.c na radku 151.
* Vlozi novy argument do tabulky symbolu.
*/
void create_arg(string *id, int type)
{
	// vytvorime uzel argumentu
	tNode *arg;
	if ((arg = malloc(sizeof(tNode))) == NULL)
		throw_err(INT_ERROR, ALL_STRUCT, 0);
	mark_mem(arg);
	strInit(&arg->key);

	// vytvorime klic pro uzel a inicializujeme ho
	strCopyString(&arg->key, id);
	arg->node_type = VARIABLE;
	arg->functions = NULL;
	arg->variables = NULL;
	arg->l_node = NULL;
	arg->r_node = NULL;
	arg->type = type;

    // zapiseme poradi argumentu a jejich pocet
	numb_arg++;
	arg->argument = numb_arg;
	tree->active_func->argument = numb_arg;

    // ve funkci neexistuji promenne -> ulozime jako prvni promennou
	if (tree->active_func->variables == NULL)
		tree->active_func->variables = arg;
	else
		add_node(arg, tree->active_func->variables);
}

/** Kopie
* Original se nachazi ve souboru table.c na radku 182.
* Funkce vyhledava argumenty funkci podle parametru.
*/
tNode *get_argument(tNode *begin, int number)
{
	if (begin == NULL)
		return NULL;

	if (begin->node_type == VARIABLE)
	{
        // nasli vhodny argument
		if (begin->argument == number)
		      return begin;
	}
    // vyhledavame v levem podstromu
	tNode *found = get_argument(begin->l_node, number);
	if (found == NULL)
    // pokud nenasli -> jdeme do praveho
		found = get_argument(begin->r_node, number);

	return found;
}

/** Kopie
* Original se nachazi ve souboru table.c na radku 203.
* Vlozi novy uzel do tabulky symbolu.
*/
void add_node(tNode *new, tNode *begin)
{
	if (begin == NULL)
		throw_err(INT_ERROR, 0, 0);

	// porovname klice
	int cmp = strCmpString(&new->key, &begin->key);
	// jdeme do leveho podstromu
	if (cmp < 0)
	{
		if (begin->l_node == NULL)
		{				// udelame novy prvek levym podstromem
			begin->l_node = new;

			if (new->node_type == CLASS)
				tree->active_class = new;	// udelame ho aktivnim

			else if (new->node_type == FUNCTION)
				tree->active_func = new;
		}
		else
			add_node(new, begin->l_node);	// nebo jdeme dal po levemy podstromu
	}
	// jdeme do praveho podstromu
	else if (cmp > 0)
	{
		if (begin->r_node == NULL)
		{				// udelame novy prvek pravym podstromem
			begin->r_node = new;
			if (new->node_type == CLASS)
				tree->active_class = new;	// udelame ho aktivnim
			else if (new->node_type == FUNCTION)
				tree->active_func = new;
		}
		else
			add_node(new, begin->r_node);	// nebo jdeme dal po pravemu podstromu
	}
	else
		throw_err(INT_ERROR, 0, 0);
}
/** Kopie
* Original se nachazi ve souboru table.c na radku 245.
* Funkce inicializuje novy strom.
*/
void tree_init(tTree *new_tree)
{
    // nastavime ukazatel na strom
	tree = new_tree;
	tree->root = NULL;
	tree->active_func = NULL;
	tree->active_class = NULL;
}
/** Kopie
* Original se nachazi ve souboru table.c na radku 255.
* Zruseni tabulky.
*/
void destroy_tree(tNode *begin)
{
	if (begin == NULL)
		return;
    // zrusime strom funkci a promennych pokud je uzel ma
    if (begin->functions != NULL)
        destroy_tree(begin->functions);
    if (begin->variables != NULL)
        destroy_tree(begin->variables);
    // jdeme dal po stromu
	if (begin->l_node != NULL)
		destroy_tree(begin->l_node);
	if (begin->r_node != NULL)
		destroy_tree(begin->r_node);
    // funkce je na terminalnim uzlu -> uvolnime jeho nazev a samotny uzel
	strFree(&begin->key);
	free_pointer(begin, true);
}

/** Kopie
* Original se nachazi ve souboru built-in.c na radku 261.
* F-ce pro razeni. Pouzit algoritmus Shell sort.
* @param str    -> Retezec, ktery je treba seradit.
* @return       -> Serazeny retezec.
**/
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
/** Kopie
* Original se nachazi ve souboru built-in.c na radku 280.
* Pomocna f-ce pro vypocet skoku.
* @param stop_sbs   -> abeceda znaku.
* @param pattern    -> vzorec, pro ktery je treba pocitat.
* @return           -> nic.
**/
void create_stop_symbols(int *stop_sbs, char *pattern) {
    for (int i = 0; i < ALPHABET_LEN; i++) {
        stop_sbs[i] = strlen(pattern);
    }
    for (int i = 0; (unsigned)i < strlen(pattern) - 1; i++) {
        stop_sbs[(unsigned char)pattern[i]] = i;
    }
}
/** Kopie.
* F-ce vyhledavani podretezce v retezci. Pouzit Boyer-Mooruv algoritmus.
* Original se nachazi ve souboru built-in.c na radku 296.
* @param s          -> Text, ve kterem se bude vyhledavat pattern.
* @param pattern    -> Vzorec, ktery se bude vyhledavat.
* @return           -> Vrati index prvniho vyskytu patternu nebo -1 pri neuspechu.
**/
int find(char *s, char *pattern)
{
	int s_len = strlen(s);
	int pat_len = strlen(pattern);

	if (pat_len == 0)
		return 0;
	if (pat_len > s_len || s_len <= 0 || pat_len < 0 || !s || !pattern)
		return NOT_FOUND;

    int stop_symbols[ALPHABET_LEN];
	create_stop_symbols(stop_symbols, pattern);

	int p_pos = pat_len - 1;
	int s_pos = p_pos;
	while (s_pos <= s_len && p_pos >= 0) {
		if (s[s_pos] == pattern[p_pos]) {
			p_pos--;
			s_pos--;
		}
		else {
            s_pos += stop_symbols[(unsigned char)s[s_pos]];
			p_pos = pat_len - 1;
		}
	}
	if (p_pos < 0)
		return ++s_pos;
	else
		return NOT_FOUND;
}
