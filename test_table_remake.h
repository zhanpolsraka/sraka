#ifndef _TABLE_H_
#define _TABLE_H_

/*	Typy uzlu v tabulce symbolu		*/
#define VARIABLE 0
#define FUNCTION 1
#define CLASS 2

/*	Struktura uzlu */
typedef struct t_node{

    // nazev uzlu
	string key;
    // v promennych oznacuje typ promenne
    // ve funkcich oznacuje typ navratove hodnoty
    int type;
    // typ uzlu
	int node_type;

    // v promennych oznacuje poradi pokud promenna je argumentu
    // ve funkcich oznacuje pocte argumentu
	int argument;

    // ukazatel na strom funkci u trid
    struct t_node *functions;
    // ukazatel na strom promennych u funkcich/trid
	struct t_node *variables;

	struct t_node *l_node;	// ukazatel na levy podstrom
	struct t_node *r_node;	// ukazatel na pravy podstrom

}tNode;

/*	Ridici struktura stromu */
typedef struct{

	tNode *root;
	tNode *active_func;
	tNode *active_class;

}tTree;

void create_node(string *id, int type, int n_type, bool stat);
tNode *get_node(string *key, int n_type, tNode *begin);
void create_arg(string *id, int type);
tNode *get_argument(tNode *begin, int number);
void add_node(tNode *new, tNode *begin);

void tree_init(tTree *new_tree);
void destroy_tree(tNode *begin);

#endif
