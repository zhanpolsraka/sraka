#ifndef _TABLE_H_
#define _TABLE_H_

/*	Typy uzlu v tabulce symbolu		*/
typedef enum{

	VARIABLE,
	FUNCTION,
	CLASS

}NodeType;

/*	Sjednoceni s hodnotou promenne, pouzivame pro ekonomie pameti	*/
union v_var{
	int integer;
	double real;
	string str;
	bool boolean;
};

/*	Informace promenne	*/
typedef struct{

	int type;			// typ promenne
	union v_var value;	// hodnota promenne

}tData;

/*	Struktura uzlu */
typedef struct t_node{

	string key;
	NodeType type;

	union{
		int argument;	// cislo argumentu pro zachovani poradi argumentu
		struct t_node *functions;	// pro tridy
	};

	// ve funkci obsahuje typ navratove hodnoty,
	// v promennych obsahuje typ a hodnotu promenne,
	tData data;

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

void tree_init(tTree *new_tree);
bool create_arg(string *id, int type);
bool add_arg(tNode *arg, tNode *begin);
bool create_node(string *id, NodeType type, int v_type, bool stat);
bool add_node(tNode *new, tNode *begin);
tNode *get_node(string *key, NodeType type, tNode *begin);
void destroy_tree(tNode *begin);
tNode *get_argument(tNode *function, int number);
void print_tree(tNode *start);

#endif
