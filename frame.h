#ifndef _FRAME_H_
#define _FRAME_H_

/*	Sjednoceni s hodnotou promenne, pouzivame pro ekonomie pameti	*/
union v_var{
	int integer;
	double real;
	string str;
	bool boolean;
};

typedef struct{

	int type;		   // typ promenne
    union v_var value; // hodnota promenne
    bool can_free;	   // flag moznosti uvolneni

}tData;

// polozka promenne v ramce
typedef struct t_var{

    string *name_var;  // nazev promenne
	tData data;	   	   // typ a hodnota promenne
    int arg_order;     // poradi promenne jako argumentu, 0 pokud neni argument
    bool is_def;       // flag definici
    bool is_init;      // flag inicializace

    struct t_var *next;// ukazatel na pristi polozku

}tVar;

// seznam polozek promennych pro dany ramec
typedef struct{

    string *name_frame; // nazev ramce
	char *name_par;	// pro funkci, jmeno tridy obsahujici funkci
    int type;           // typ ramce, muze byt tridou nebo funkci
	int ret_type;		// typ navratove hodnoty pro ramci funkci
    int numb_arg;       // pocet argumentu ktery ma funkce
    int last_pozition;  // index polozky, ze ktere byl vytvoren novy ramec

    tVar *Act;      // aktivni seznam
    tVar *First;    // prvni seznam

}tListData;

// zasobnik ramcu
typedef struct{

    tListData **stack;
    int top;            // vrchol zasobniku
    int alloc_size;     // alokovane misto pro ukazatele

}tFrameStack;


tFrameStack *create_frame_stack();
void create_frame(tFrameStack *st, tNode *node, char *name_class, int *arg_count);
void set_ret_indx(int indx);
void destroy_frame(tFrameStack *st);

void frame_stack_push(tFrameStack *st, tListData *list);
void fill_list(tListData *list, tNode *func, bool is_class);
void insert_to_list(tListData *list, tVar *var);

void print_frames(tFrameStack *st);
int get_ret_val();
tData *get_value(tFrameStack *st, string *name, bool init);
tData *get_arg(int number);
void def_var(string *name);

#endif
