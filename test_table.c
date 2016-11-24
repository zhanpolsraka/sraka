
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "str.h"
#include "test_scanner.h"
#include "test_error.h"
#include "test_table.h"

// Strom
tTree *tree;
// Cislo argumentu
int numb_arg = 0;

/*	Testovaci funkce vypise strom	*/
void print_tree(tNode *start)
{
	if (start == NULL) start = tree->root;
	//printf("%s %s", "This is the node with key", strGetStr(&start->key));
	if (start->type == CLASS)
	{
		printf("Class %s\n", strGetStr(&start->key));
		if (start->variables != NULL)
		{
			print_tree(start->variables);
		}
		if (start->functions != NULL)
		{
			print_tree(start->functions);
		}
	}
	else if (start->type == FUNCTION)
	{
		printf("	Function -> %s\n", strGetStr(&start->key));
		if (start->variables != NULL)
		{
			printf("Lokal variables: \n");
			print_tree(start->variables);
		}
	}
	else
	{
		printf("	Variable -> %s", strGetStr(&start->key));
		if (start->data.type == INT)
			printf(", type INTEGER, value [%d]\n", start->data.value.integer);
		else if (start->data.type == DOUBLE)
			printf(", type DOUBLE, value [%f]\n", start->data.value.real);
		else if (start->data.type == BOOLEAN)
			printf(", type BOOLEAN, value [%d]\n", start->data.value.boolean);
		else if (start->data.type == STRING)
			printf(", type STRING, value [%s]\n", strGetStr(&start->data.value.str));
	}

	if (start->l_node != NULL)
	{
		print_tree(start->l_node);
	}
	if (start->r_node != NULL)
	{
		print_tree(start->r_node);
	}
}

/*	Funkce inicializuje novy strom */
void tree_init(tTree *new_tree)
{
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
	switch(begin->type)
	{
		case VARIABLE:
			break;

		case FUNCTION:
			if (begin->variables != NULL)
				destroy_tree(begin->variables);
			break;

		case CLASS:
			if (begin->functions != NULL)
				destroy_tree(begin->functions);
			if (begin->variables != NULL)
				destroy_tree(begin->variables);
			break;
	}
	if (begin->l_node != NULL)
		destroy_tree(begin->l_node);
	if (begin->r_node != NULL)
		destroy_tree(begin->r_node);

	strFree(&begin->key);
	free(begin);
}

/*	Hledani urcite promenne/funkci/tridy	*/
tNode *get_node(string *key, NodeType type, tNode *begin)
{
	if (tree->root == NULL)
		return NULL;
	else if (begin == NULL)
		begin = tree->root;

	if (begin != NULL)
	{
		// porovname klice
		int cmp = strCmpString(key, &begin->key);
		// jdeme do leveho podstromu
		if (cmp < 0)
		{
			if (begin->l_node != NULL)
			{
				return get_node(key, type, begin->l_node);
			}
			else
				return NULL;
		}
		// jdeme do praveho podstromu
		else if (cmp > 0)
		{
			if (begin->r_node != NULL)
			{
				return get_node(key, type, begin->r_node);
			}
			else
				return NULL;
		}
		// nasli uzel
		else if (cmp == 0 && begin->type == type)
			return begin;
		else
			throw_err(PR_ERROR, TREE_ERROR);
	}
	return NULL;
}

/*	Vlozi novy uzel do tabulky symbolu	*/
void add_node(tNode *new, tNode *begin)
{
	if (begin == NULL)
		throw_err(PR_ERROR, TREE_ERROR);

	// porovname klice
	int cmp = strCmpString(&new->key, &begin->key);
	// jdeme do leveho podstromu
	if (cmp < 0)
	{
		if (begin->l_node == NULL)
		{				// udelame novy prvek levym podstromem
			begin->l_node = new;

			if (new->type == CLASS)
				tree->active_class = new;	// udelame ho aktivnim

			else if (new->type == FUNCTION)
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
			if (new->type == CLASS) tree->active_class = new;	// udelame ho aktivnim
			else if (new->type == FUNCTION) tree->active_func = new;
		}
		else
			add_node(new, begin->r_node);	// nebo jdeme dal po pravemu podstromu
	}
	else
		throw_err(PR_ERROR, TREE_ERROR);
}

/*	Vytvori, inicializuje a vlozi novy uzel do tabulky symbolu	*/
void create_node(string *id, NodeType type, int v_type, bool stat)
{
	// pokud pred operaci se zpracovavali argumenty, nastavime pocet argumentu na nulu
	if (numb_arg)
		numb_arg = 0;
	// nazev nesmi obsahovat tecku
	if (strchr(strGetStr(id), '.') != NULL)
		throw_err(SYN_ERROR, UNK_EXPR);

	// vytvorime novy uzel
	tNode *new;
	if ((new = malloc(sizeof(tNode))) == NULL)
		throw_err(ALLOC_ERROR, ALL_STRUCT);

	strInit(&new->key);
	// vytvorime klic pro uzel a inicializujeme ho
	strCopyString(&new->key, id);
	new->type = type;
	new->l_node = NULL;
	new->r_node =NULL;

	// na zaklade typu uzlu urcime kde bude zacatek
	tNode *begin;
	switch (new->type)
	{
		case VARIABLE:
			new->data.type = v_type;// pokud uzel bude promennou nastavime jeji typ
			new->variables = NULL;	// nepouzijeme pro promenne
			if (stat) begin = tree->active_class->variables;
			else begin = tree->active_func->variables;
		break;

		case FUNCTION:
			new->data.type = v_type;// pokud uzel bude funkci nastavime typ navratove hodnoty
			new->variables = NULL;
			begin = tree->active_class->functions;
		break;

		case CLASS:
			new->functions = NULL;
			new->variables = NULL;
			begin = tree->root;
		break;
	}
	// neexistuje korenovy uzel
	if (tree->root == NULL && new->type == CLASS)
	{
		tree->root = new;
		tree->active_class = new;
	}
	// ve tride neexistuji funkce
	else if (new->type == FUNCTION && tree->active_class->functions == NULL)
	{
		tree->active_class->functions = new;
		tree->active_func = new;
	}
	// ve tride neexistuji staticke promenne
	else if (new->type == VARIABLE && stat && tree->active_class->variables == NULL)
	{
		tree->active_class->variables = new;
	}
	// ve funkci neexistuji promenne
	else if (new->type == VARIABLE && tree->active_func != NULL &&
		tree->active_func->variables == NULL && !stat)
		{
			tree->active_func->variables = new;
	}
	else
		add_node(new, begin);
}

/*	Vlozi novy argument do tabulky symbolu	*/
void create_arg(string *id, int type)
{
	// vytvorime uzel argumentu
	tNode *arg;
	if ((arg = malloc(sizeof(tNode))) == NULL)
		throw_err(ALLOC_ERROR, ALL_STRUCT);

	strInit(&arg->key);
	// vytvorime klic pro uzel a inicializujeme ho
	strCopyString(&arg->key, id);
	arg->type = VARIABLE;
	arg->l_node = NULL;
	arg->r_node = NULL;
	arg->data.type = type;		// nastavime typ argumentu

	numb_arg++;
	arg->argument = numb_arg;
	tree->active_func->data.value.integer = numb_arg;

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

	if (begin->type == VARIABLE)
	{
		if (begin->argument == number)
		return begin;
	}

	tNode *found = get_argument(begin->l_node, number);
	if (found == NULL)
		found = get_argument(begin->r_node, number);

	return found;
}
