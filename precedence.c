/* **************************************************************************/
/* Projekt:             Implementace interpretu jazyka IFJ16				*/
/* Predmet:             Formalni jazyky a prekladace (IFJ)					*/
/* Soubor:              precedence.c  (Precedencni analyza)					*/
/*																			*/
/* Autor login:      	Ermak Aleksei		xermak00						*/
/*                     	Khaitovich Anna		xkhait00						*/
/*						Nesmelova Antonina	xnesmel00						*/
/*						Fedorenko Oleh		xfedor07						*/
/*						Fedin Evgenii		xfedin00						*/
/* **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "str.h"
#include "scanner.h"
#include "error.h"
#include "table.h"
#include "parser.h"
#include "precedence.h"
#include "instructions.h"
#include "frame.h"
#include "interpret.h"
#include "built_in.h"
#include "buffer.h"

bool cond_expr = false;
int arg_expr = 0;

/* Precedencni tabulka	*/
int pr_table[][20] = {
	/* 		  id  +	  -   *   /   ;	 val  (   )  ==   <   >   <=  >=  !=  !  ,  {   ||  &&*/
	/*id */{  0,  1,  1,  1,  1,  1,  0,  2,  1,  1,  1,  1,  1,  1,  1,  0, 1,  0,  1,  1},
	/* + */{ -1,  1,  1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  0,  1, -1},
	/* - */{ -1,  1,  1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  0,  1, -1},
	/* * */{ -1,  1,  1,  1,  1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  0,  1,  1},
	/* / */{ -1,  1,  1,  1,  1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  0,  1,  1},
	/* ; */{ -1, -1, -1, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, 0,  0, -1, -1},
 	/*val*/{  0,  1,  1,  1,  1,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1, 1,  0,  1,  1},
	/* ( */{ -1, -1, -1, -1, -1,  0, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, 0,  0, -1, -1},
	/* ) */{  0,  1,  1,  1,  1,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1, 1,  1,  1,  1},
	/*== */{ -1, -1, -1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0, -1, -1},
	/* < */{ -1, -1, -1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0, -1, -1},
	/* > */{ -1, -1, -1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0, -1, -1},
	/*<= */{ -1, -1, -1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0, -1, -1},
	/*>= */{ -1, -1, -1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0, -1, -1},
	/*!= */{ -1, -1, -1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0, -1, -1},
	/* ! */{ -1,  1,  1,  1,  1,  1, -1,  0,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0,  1,  1},
	/* , */{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0},
	/* { */{  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0,  0,  0},
	/*|| */{ -1,  1,  1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  0,  1, -1},
	/*&& */{ -1,  1,  1,  1,  1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 1,  0,  1,  1}
};

/*	Funkce zpracovani vyrazu	*/
void expression(Token *token, string *target)
{
    // handle
    tItem *handle[3];
    // vytvorime zasobniky pro praci s vyrazy
    tStack *main_st = NULL;
    tStack *help_st = NULL;

	if ((main_st = malloc(sizeof(tStack))) == NULL ||
        (help_st = malloc(sizeof(tStack))) == NULL)
		throw_err(INT_ERROR, ALL_STRUCT, 0);

	mark_mem(main_st);
	mark_mem(help_st);
	stack_init(main_st);
	stack_init(help_st);

    // hodnota, ktera urcuje na zaklade precedencni tabulki jakou akci provadime
    int action;
    // pomocna promenna, hodnota ktera rika jestli muzeme cist a vkladat na zasobnik dalsi token
    bool read_next = true;
	// inicializujeme a vkladame na vrchol zasobniku ukoncovaci symbol
	tItem *end_item;
	end_item = init_item();

	end_item->data->type = SEMICOLON;
	end_item->i_type = TERM;

	stack_push(main_st, end_item);

	// inicializujeme token reprezentujici vstupni data
	tItem *entery;
	while(1)
    {   // cteme pristi token jenom kdyz program ne zpracovava handle
		if (read_next)
        {
			entery = get_data(token);
		}
		// vyjdeme z cyklu bez chyby jestli pristi token je ukoncovac
		// nebo prava hranata zavorka pri podmince
		//printf("main_st->top == 1 --> %d\n", main_st->top == 1);
		//printf("get_top_term(main_st) == SEMICOLON -- >%d\n", get_top_term(main_st) == SEMICOLON);
		//printf("!cond_expr -- > %d\n", !cond_expr);
		//printf("!arg_expr -- > %d\n", !arg_expr);
		if ((/*main_st->top == 1 && */get_top_term(main_st) == SEMICOLON &&
			(((entery->data->type == SEMICOLON && !cond_expr && !arg_expr) ||
			(entery->data->type == L_VIN && cond_expr)) ||
			((entery->data->type == COMMA || entery->data->type == R_PAR) && arg_expr))))
		{
			if (cond_expr)
				cond_expr = false;
			else if (arg_expr)
				arg_expr--;
			else if (target)
				create_instruction(ASSIGNMENT, NULL, NULL, target);
			break;
		}
		//print_stack(main_st);
		// kontrola aby vstupni data byly platnymi pro vyraz
		if (entery->data->type > 19)
		{
			if (equal_str(entery->data->attr.str, "true") ||
				equal_str(entery->data->attr.str, "false"))
				entery->data->type = VALUE;
			else
				throw_err(SYN_ERROR, UNK_EXPR, entery->data->attr.str);
		}

		// kontrola volani funkci ve vyrazu: za id jde leva zavorka
		if (get_top_term(main_st) == IDENTIFIER && entery->data->type == L_PAR)
		{
			call_func_expr(token, &main_st->stack[main_st->top]->data->attr);
		}
		//printf("entery %d %s\n", entery->data->type, strGetStr(&entery->data->attr));
		//printf("top %d\n", get_top_term(main_st));
		// urcime akce kterou provadime
		action = pr_table[get_top_term(main_st)][entery->data->type];
		//printf("action %d\n", action);
		switch (action)
        {   // zacatek handlu
			case PR_LESS:
                // muzeme cist dalsi token
				read_next = true;
                // vlozime < do zasobniku
				tItem *d_board;
				d_board = init_item();
				d_board->i_type = DOWN_BOARD;

				// premistime vsechna data do pomocneho zasobniku
				shift_elem_to_term(main_st, help_st);
				// vlozime na vrchol hlavniho zasobniku znak <
				stack_push(main_st, d_board);
				// vratime puvodni obsah hlavniho zasobniku
				shift_all_elem(help_st, main_st);
				// vlozime vstupni terminal na vrchol zasobniku
				stack_push(main_st, entery);

				//print_stack(main_st);
				break;
            // konec handlu
			case PR_GRTR:
                // zatim necteme dalsi token
				read_next = false;
				// udelame novy vyraz
				turn_to_expr(main_st, handle);
				// sgenerujeme instrukce
				generate_instr(handle);
				//print_stack(main_st);
				break;
            // vlozime novou polozku do zasobniku
			case PR_EQUAL:
				read_next = true;
				stack_push(main_st, entery);
				//print_stack(main_st);
				break;
            // nemame pravidel pro vyraz
			case PR_NULL:
				//printf("----> precedence, expression 2\n");
				throw_err(SYN_ERROR, UNK_EXPR, entery->data->attr.str);
				break;
		}
	}
	//printf("%s\n", "done!");
	// uvolneme stack a strukturu pro vstupni data
	free_item(entery);
	free_stacks(main_st, help_st);
}

/* 	Funkce zpracuje handle	*/
void turn_to_expr(tStack *m_st, tItem *handle[])
{   // pocitadlo
	int i = 0;
    // pomocna struktura
	tItem *help;
    // naplnime handle
	while ((m_st->stack[m_st->top])->i_type != DOWN_BOARD)
	{	// pokud pocet elementu v handlu je vetsi nez 3, vyvola chybu
		if (i > 2)
			throw_err(SYN_ERROR, UNK_EXPR, "no token");

		help = stack_pop(m_st);
		handle[i] = help;
		i++;
	}
	// odebereme z hlavniho zasobniku polozku se znakem <
	help = stack_pop(m_st);
	// a zmenime ji na EXPR
	help->i_type = EXPR;

	// vyhledame pravidlo pro handle
	if (handle[0]->i_type == TERM &&
		(handle[0]->data->type == IDENTIFIER || handle[0]->data->type == VALUE))
	{   // pro hodnoty a identifikatory ulozime jejich nazev
		strCopyString(&help->data->attr, &handle[0]->data->attr);
	}
    else
    {   // cely vyraz se bude pak vyhledavat v zasobniku mezivysledku
        strAddChar(&help->data->attr, 'e'); // e -> expression
    }
	// vlozime na vrchol hlavniho zasobniku EXPR
	stack_push(m_st, help);
}

/*	Funkce sgeneruje instrukce pro handle	*/
void generate_instr(tItem *handle[])
{
	if (handle[0]->i_type == EXPR && handle[1]->data->type == NOT)
    {
		create_instruction(NOT, NULL, NULL, NULL);
		free_handle(handle, 2);
	}
	else if (handle[0]->i_type == EXPR && handle[2]->i_type == EXPR)
    {
		if ((handle[1]->data->type >= PLUS && handle[1]->data->type <= DIV) ||
			(handle[1]->data->type >= COMPARISON && handle[1]->data->type <= NEQ) ||
			(handle[1]->data->type >= OR && handle[1]->data->type <= AND))
        {
			create_instruction(handle[1]->data->type, NULL, NULL, NULL);
			free_handle(handle, 3);
		}
	}
	else if (handle[0]->data->type == R_PAR && handle[1]->i_type == EXPR &&
		handle[2]->data->type == L_PAR)
		{
			free_handle(handle, 3);
		}
	else if (handle[0]->data->type == R_PAR && handle[1]->data->type == L_PAR &&
			handle[2]->data->type == IDENTIFIER)
		{
			if (equal_str(handle[2]->data->attr.str, "ifj16.readInt"))
				create_instruction(INSTR_R_INT, NULL, NULL, NULL);
			else if (equal_str(handle[2]->data->attr.str, "ifj16.readDouble"))
				create_instruction(INSTR_R_DOUBLE, NULL, NULL, NULL);
			else if (equal_str(handle[2]->data->attr.str, "ifj16.readString"))
				create_instruction(INSTR_R_STRING, NULL, NULL, NULL);
			else if (equal_str(handle[2]->data->attr.str, "ifj16.length"))
				create_instruction(INSTR_LENGTH, NULL, NULL, NULL);
			else if (equal_str(handle[2]->data->attr.str, "ifj16.substr"))
				create_instruction(INSTR_SUBSTR, NULL, NULL, NULL);
			else if (equal_str(handle[2]->data->attr.str, "ifj16.compare"))
				create_instruction(INSTR_COMPARE, NULL, NULL, NULL);
			else if (equal_str(handle[2]->data->attr.str, "ifj16.sort"))
				create_instruction(INSTR_SORT, NULL, NULL, NULL);
			else if (equal_str(handle[2]->data->attr.str, "ifj16.find"))
				create_instruction(INSTR_FIND, NULL, NULL, NULL);
			else
				create_instruction(INSTR_CALL_EXP_FUNC, NULL, NULL, &handle[2]->data->attr);
			free_handle(handle, 3);
		}
	else if(handle[0]->data->type == IDENTIFIER || handle[0]->data->type == VALUE)
	{
		if (handle[0]->data->type == IDENTIFIER)
			// addr1 - nazev promenne
			create_instruction(INSTR_INSERT, &handle[0]->data->attr, NULL, NULL);
		else
			// addr2 - hodnota
			create_instruction(INSTR_INSERT, NULL, &handle[0]->data->attr, NULL);

		free_handle(handle, 1);
	}
	else
		throw_err(SYN_ERROR, UNK_EXPR, "no token");
}

void call_func_expr(Token *token, string *str)
{
	// precteme prvni argument
	get_token(token);
	if (token->type == R_PAR)
	{
		if (is_built_in(str->str))
				throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG, str->str);
		go_back(token);
		if (!equal_str(str->str, "ifj16.readInt") &&
			!equal_str(str->str, "ifj16.readDouble") &&
			!equal_str(str->str, "ifj16.readString"))
			// vytvorime ramec promennych pokud volana funkce neni vestavena
			create_instruction(CREATE_FRAME, str, NULL, NULL);
		return;
	}
	go_back(token);

	if (equal_str(str->str, "ifj16.readInt") ||
		equal_str(str->str, "ifj16.readDouble") ||
		equal_str(str->str, "ifj16.readString"))
		throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG, str->str);

	int n_arg = 0;
	while (token->type != R_PAR)
	{
		arg_expr++;
		n_arg++;
		expression(token, NULL);
		// length a sort potrebuji jeden argument
		if ((equal_str(str->str, "ifj16.length") && token->type != R_PAR) ||
			(equal_str(str->str, "ifj16.sort") && token->type != R_PAR) ||
			(equal_str(str->str, "ifj16.substr") && token->type == R_PAR && n_arg != 3) ||
			(equal_str(str->str, "ifj16.compare") && token->type == R_PAR && n_arg != 2) ||
			(equal_str(str->str, "ifj16.find") && token->type == R_PAR && n_arg != 2) ||
			(token->type != COMMA && token->type != R_PAR))
			throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG, str->str);
	}
	// vytvorime ramec promennych pokud volana funkce neni vestavena
	if (!is_built_in(str->str))
		create_instruction(CREATE_FRAME, str, NULL, NULL);
	// vytvorime instrukci prirazeni argumentum
	if (!is_built_in(str->str))
	{
		for (; n_arg != 0; n_arg--)
			create_instruction(INSTR_ASS_ARG, NULL, NULL, str);
	}
	// vratime pravou zavorku zpet
	go_back(token);
}

bool is_built_in(char *name)
{
	if (equal_str(name, "ifj16.length") ||
		equal_str(name, "ifj16.substr") ||
		equal_str(name, "ifj16.compare") ||
		equal_str(name, "ifj16.sort") ||
		equal_str(name, "ifj16.find"))
		return true;
	return false;
}

/*	Cteni dalsiho tokenu a priprava nove polozky 	*/
tItem *get_data(Token *token)
{   // vytvorime novou polozku a alokujeme novou pamet pro nej
	tItem *new_item;
	if ((new_item = init_item()) == NULL)
		throw_err(INT_ERROR, ALL_STRUCT, 0);
    // cteme dalsi token ...
	get_token(token);
	if (token->type == T_EOF)
		throw_err(SYN_ERROR, UNK_EXPR, "end of file");
    // ... a ukladame informace do polozky
	strCopyString(&new_item->data->attr, &token->attr);
	new_item->data->type = token->type;
    // udelame polozku terminalem
	new_item->i_type = TERM;
	return new_item;
}

/*	Funkce vrati typ nejhornejsiho terminalu na zasobniku	*/
tType get_top_term(tStack *st)
{
	int i = st->top;
	while ((st->stack[i])->i_type == EXPR || (st->stack[i])->i_type == DOWN_BOARD)
		i--;

	return (st->stack[i])->data->type;
}

/* Funkce premisti elementy z jedneho zasobniku do jineho do prvniho terminalu	*/
void shift_elem_to_term(tStack *src, tStack *dst)
{
	tItem *help;
	while ((src->stack[src->top])->i_type == EXPR || (src->stack[src->top])->i_type == DOWN_BOARD)
    {
		help = stack_pop(src);
		if (help == NULL)
			throw_err(INT_ERROR, 0, 0);

		stack_push(dst, help);
	}
}

/*	Funkce premisti vsechny elementy z jedneho zasobniku do jineho	*/
void shift_all_elem(tStack *src, tStack *dst)
{
	tItem *help;
	while (!stack_empty(src))
    {
		help = stack_pop(src);
		if (help == NULL)
			throw_err(INT_ERROR, 0, 0);

		stack_push(dst, help);
	}
}

/*	Funkce inicializuje novou polozku seznamu */
tItem *init_item()
{
	tItem *new_item;
	if ((new_item = malloc(sizeof(tItem))) == NULL)
		throw_err(INT_ERROR, ALL_STRUCT, 0);
	mark_mem(new_item);

	Token *new_token;
	if ((new_token = malloc(sizeof(Token))) == NULL)
		throw_err(INT_ERROR, ALL_STRUCT, 0);
	mark_mem(new_token);
	strInit(&new_token->attr);
	new_item->data = new_token;
	return new_item;
}

/*	Funkce uvolni polozku staku	*/
void free_item(tItem *item)
{
	strFree(&item->data->attr);
	free_pointer(item->data, true);
	free_pointer(item, true);
}

/*	Funkce uvolni handle	*/
void free_handle(tItem **handle, int num)
{
	for (int i = 0; i < num; ++i)
    {
		if (handle[i]->data != NULL)
			free_token(handle[i]->data);

		free_pointer(handle[i], true);
	}
}

/*	Inicializace zasobniku	*/
void stack_init(tStack *st)
{
	st->top = -1;
	st->stack_size = DEF_STACK_SIZE;
	if ((st->stack = malloc(st->stack_size * sizeof(tItem *))) == NULL)
		throw_err(INT_ERROR, ALL_STRUCT, 0);
	mark_mem(st->stack);
}

/*	Funkce uklada data na vrchol zasobniku	*/
void stack_push(tStack *st, tItem *data)
{
	if (!stack_full(st))
    {
		st->top++;
		st->stack[st->top] = data;
	}
	else
    {
		st->stack_size += 1;
		free_pointer(st->stack, false);
		if ((st->stack = realloc(st->stack, st->stack_size * sizeof(tItem *))) == NULL)
			throw_err(INT_ERROR, ALL_STRUCT, 0);
		mark_mem(st->stack);
		stack_push(st, data);
	}
}

/*	Funkce odstrani polozku z vrcholu zasobniku a vrati ukazatel na nej 	*/
tItem *stack_pop(tStack *st)
{
	tItem *ret;
	if (!stack_empty(st))
    {
		ret = st->stack[st->top];
		st->stack[st->top] = NULL;
		st->top--;
		return ret;
	}
	return  NULL;
}

/*	Funkce kontroluje je-li zasobnik plny	*/
bool stack_full(tStack *st)
{
    return st->top == st->stack_size-1;
}

/*	Funkce kontroluje je-li zasobnik prazdny	*/
bool stack_empty(tStack *st)
{
    return st->top == -1;
}

/*  Funkce uvolni pamet zasobniku*/
void free_stacks(tStack *main_st, tStack *help_st)
{
	stack_destroy(main_st);
	free_pointer(main_st->stack, true);
	free_pointer(main_st, true);
	stack_destroy(help_st);
	free_pointer(help_st->stack, true);
	free_pointer(help_st, true);
}

/*	Zniceni polozek zasobniku 	*/
void stack_destroy(tStack *st)
{
	if (!stack_empty(st))
    {
		for (int i = st->top; i > -1; --i)
        {
			if ((st->stack[i])->data != NULL)
            {
				strFree(&(st->stack[i])->data->attr);
				free_pointer(st->stack[i]->data, true);
			}
			free_pointer(st->stack[i], true);
		}
	}
}

/*	Testovaci funkce, obsah zasobniku	*/
void print_stack(tStack *st)
{
	printf("\n\n");
	for (int i = st->top; i > -1; --i)
    {
		printf("~~stack item %d: type %d~~", i, (st->stack[i])->i_type);
		if ((st->stack[i])->i_type == DOWN_BOARD)
			printf(" %s", "<");

		else if ((st->stack[i])->i_type == EXPR)
			printf(" %s %s", "expr", strGetStr(&(st->stack[i])->data->attr));

		else if (st->stack[i]->data != NULL)
			printf("%s %d", strGetStr(&(st->stack[i])->data->attr), (st->stack[i])->data->type);

		else printf(" %d", (st->stack[i])->i_type);
		printf("\n\n");
	}
}
