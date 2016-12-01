#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "str.h"
#include "test_scanner.h"
#include "test_error.h"
#include "test_table_remake.h"
#include "buffer.h"

// globalni ukazatel na strom
tTree *tree;
// pocet argumentu funkci
int numb_arg = 0;

/*	Vytvori, inicializuje a vlozi novy uzel do tabulky symbolu	*/
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

/*	Hledani urciteho uzlu podle nazvu	*/
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

/*	Vlozi novy argument do tabulky symbolu	*/
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

/*	Funkce vyhledava argumenty funkci podle parametru	*/
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

/*	Vlozi novy uzel do tabulky symbolu	*/
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

/*	Funkce inicializuje novy strom */
void tree_init(tTree *new_tree)
{
    // nastavime ukazatel na strom
	tree = new_tree;
	tree->root = NULL;
	tree->active_func = NULL;
	tree->active_class = NULL;
}

/*	Zruseni tabulky		*/
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
