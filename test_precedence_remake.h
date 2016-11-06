#ifndef _PRECEDENCE_H_
#define _PRECEDENCE_H_

// velikost zasobniku
#define DEF_STACK_SIZE 20
// znaky terninalu a neterminalu
#define EXPR 1
#define DOWN_BOARD 2
#define TERM 3

extern bool cond_expr;

/* Znaky hranic handlu	*/
typedef enum{

	PR_GRTR = 1,	// >
	PR_LESS = -1,	// <
	PR_EQUAL = 2,	// =
	PR_NULL = 0		// 0

}tPrecedence;

/*	Struktura polozky zasobniku	*/
typedef struct{

	Token *data;	// ukazatel na precteny token
	int i_type;		// typ polozky: terminal nebo neterminal

}tItem;

/*	Struktura zasobniku	*/
typedef struct{

	tItem **stack;
	int top;
	int stack_size;

}tStack;

bool expression(Token *token, string *target);
bool turn_to_expr(tStack *m_st, tItem *handle[]);
bool generate_instr(tItem *handle[]);
bool call_func_expr(Token *token, string *str);
void free_handle(tItem **handle, int num);
bool shift_all_elem(tStack *src, tStack *dst);
bool shift_elem_to_term(tStack *src, tStack *dst);
tType get_top_term(tStack *st);
tItem *get_data(Token *token);
tItem *init_item();
void free_item(tItem *item);

bool stack_push(tStack *st, tItem *data);
tItem *stack_pop(tStack *st);
void free_stacks();
void stack_destroy(tStack *st);
bool stack_full(tStack *st);
bool stack_empty(tStack *st);
bool stack_init(tStack *st);
void print_stack(tStack *st);

#endif
