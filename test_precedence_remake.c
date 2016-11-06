#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "str.h"
#include "test_scanner.h"
#include "test_parser_remake.h"
#include "test_precedence_remake.h"
#include "test_inst.h"
#include "test_error.h"

bool cond_expr;
bool arg_expr;

/* Precedencni tabulka	*/
int pr_table[][18] = {
	/* 		  id  +	  -   *   /   ;	 val  (   )  ==   <   >   <=  >=  !=  !  ,  {*/
	/*id */{  0,  1,  1,  1,  1,  1,  0,  2,  1,  1,  1,  1,  1,  1,  1,  0, 1,  0},
	/* + */{ -1,  1,  1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0},
	/* - */{ -1,  1,  1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0},
	/* * */{ -1,  1,  1,  1,  1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0},
	/* / */{ -1,  1,  1,  1,  1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0},
	/* ; */{ -1, -1, -1, -1, -1,  0, -1, -1,  0, -1, -1, -1, -1, -1, -1, -1, 0,  0},
 	/*val*/{  0,  1,  1,  1,  1,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1, 1,  0},
	/* ( */{ -1, -1, -1, -1, -1,  0, -1, -1,  2, -1, -1, -1, -1, -1, -1, -1, 0,  0},
	/* ) */{  0,  1,  1,  1,  1,  1,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1, 0,  1},
	/*== */{ -1, -1, -1, -1, -1,  0, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0},
	/* < */{ -1, -1, -1, -1, -1,  0, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0},
	/* > */{ -1, -1, -1, -1, -1,  0, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0},
	/*<= */{ -1, -1, -1, -1, -1,  0, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0},
	/*>= */{ -1, -1, -1, -1, -1,  0, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0},
	/* ! */{ -1,  1,  1,  1,  1,  0, -1,  0,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0},
	/*!= */{ -1, -1, -1, -1, -1,  0, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, 0,  0},
	/* , */{ -1,  0,  0,  0,  0,  0, -1,  0,  0,  0,  0,  0,  0,  0,  0,  0, 0,  0},
};

/*	Funkce zpracovani vyrazu	*/
bool expression(Token *token, string *target)
{
	// detekce chyby
	bool error = false;
    // handle
    tItem *handle[3];
    // vytvorime zasobniky pro praci s vyrazy
    tStack *main_st = NULL;
    tStack *help_st = NULL;

	if ((main_st = malloc(sizeof(tStack))) == NULL || !stack_init(main_st) ||
        (help_st = malloc(sizeof(tStack))) == NULL || !stack_init(help_st))
    {
		throw_err(ALLOC_ERROR, ALL_STRUCT);
		error = true;
	}

    // hodnota, ktera urcuje na zaklade precedencni tabulki jakou akci provadime
    int action;
    // pomocna promenna, hodnota ktera rika jestli muzeme cist a vkladat na zasobnik dalsi token
    bool read_next = true;
	// inicializujeme a vkladame na vrchol zasobniku ukoncovaci symbol
	tItem *end_item;
	if ((end_item = init_item()) == NULL)
		error = true;
	if (!error)
    {
		end_item->data->type = SEMICOLON;
		end_item->i_type = TERM;
	}
	if(!stack_push(main_st, end_item))
		error = true;

	// inicializujeme token reprezentujici vstupni data
	tItem *entery;
	while(true && !error)
    {   // cteme pristi token jenom kdyz program ne zpracovava handle
		if (read_next)
        {
			entery = get_data(token);
			if (entery == NULL)
            {
				error = true;
				break;
			}
		}
		// vyjdeme z cyklu bez chyby jestli pristi token je ukoncovac
		// nebo prava hranata zavorka pri podmince
		if ((main_st->top == 1 && get_top_term(main_st) == SEMICOLON &&
			(((entery->data->type == SEMICOLON && !cond_expr && !arg_expr) ||
			(entery->data->type == L_VIN && cond_expr)) ||
			((entery->data->type == COMMA || entery->data->type == R_PAR) && arg_expr))))
		{
			if (cond_expr)
				cond_expr = false;
			else if (arg_expr)
				arg_expr = false;
			else if (target)
				create_instruction(ASSIGNMENT, NULL, NULL, target);
			break;
		}
		// kontrola aby vstupni data byly platnymi pro vyraz
		if (entery->data->type > 19)
        {
			throw_err(SYN_ERROR, UNK_EXPR);
			error = true;
			break;
		}

		// kontrola volani funkci ve vyrazu: za id jde leva zavorka
		if (get_top_term(main_st) == IDENTIFIER && entery->data->type == L_PAR)
		{
			if (!call_func_expr(token, &main_st->stack[main_st->top]->data->attr))
			{
				//printf("\nCall function unsuccessful!\n");
				error = true;
				break;
			}
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
				if ((d_board = init_item()) == NULL)
                {
					error = true;
					break;
				}
				d_board->i_type = DOWN_BOARD;

				// premistime vsechna data do pomocneho zasobniku
				if (!shift_elem_to_term(main_st, help_st) ||
				// vlozime na vrchol hlavniho zasobniku znak <
					!stack_push(main_st, d_board) ||
				// vratime puvodni obsah hlavniho zasobniku
					!shift_all_elem(help_st, main_st) ||
				// vlozime vstupni terminal na vrchol zasobniku
					!stack_push(main_st, entery))
                {
					error = true;
					break;
				}
				//print_stack(main_st);
				break;
            // konec handlu
			case PR_GRTR:
                // zatim necteme dalsi token
				read_next = false;
				// udelame novy vyraz
				if (turn_to_expr(main_st, handle))
				{	// sgenerujeme instrukce
					if (!generate_instr(handle))
					{
						error = true;
						break;
					}
				}
				else
                {
					error = true;
					break;
				}
				//print_stack(main_st);
				break;
            // vlozime novou polozku do zasobniku
			case PR_EQUAL:
				read_next = true;
				if (!stack_push(main_st, entery))
                {
					error = true;
					break;
				}
				//print_stack(main_st);
				break;
            // nemame pravidel pro vyraz
			case PR_NULL:
				error = true;
				break;
		}
	}
	// uvolneme stack a strukturu pro vstupni data
	free_item(entery);
	free_stacks(main_st, help_st);
    // vratime false pokud nastala nejaka chyba
	if(error) return false;
	return true;
}

/* 	Funkce zpracuje handle	*/
bool turn_to_expr(tStack *m_st, tItem *handle[])
{   // pocitadlo
	int i = 0;
    // pomocna struktura
	tItem *help;
    // naplnime handle
	while ((m_st->stack[m_st->top])->i_type != DOWN_BOARD)
	{	// pokud pocet elementu v handlu je vetsi nez 3, vyvola chybu
		if (i > 2)
        {
			throw_err(SYN_ERROR, UNK_EXPR);
			return false;
		}
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
	if (!stack_push(m_st, help))
		return false;

	return true;
}

/*	Funkce sgeneruje instrukce pro handle	*/
bool generate_instr(tItem *handle[])
{
	if (handle[0]->i_type == EXPR && handle[1]->data->type == NOT)
    {
		free_handle(handle, 2);
	}
	else if (handle[0]->i_type == EXPR && handle[2]->i_type == EXPR)
    {
		if ((handle[1]->data->type >= PLUS && handle[1]->data->type <= DIV) ||
			(handle[1]->data->type >= COMPARISON && handle[1]->data->type <= NEQ))
        {
			create_instruction(handle[1]->data->type, NULL, NULL, NULL);
			free_handle(handle, 3);
		}
	}
	else if (handle[0]->data->type == R_PAR && handle[1]->data->type == L_PAR &&
			handle[2]->data->type == IDENTIFIER)
		{
			create_instruction(INSTR_CALL_FUNC, NULL, NULL, &handle[2]->data->attr);
			free_handle(handle, 3);
		}
	else if (handle[0]->data->type == R_PAR && handle[1]->i_type == EXPR &&
		handle[2]->data->type == L_PAR)
	{
		free_handle(handle, 3);
	}
	else if(handle[0]->data->type == IDENTIFIER || handle[0]->data->type == VALUE)
	{
		create_instruction(INSTR_INSERT, &handle[0]->data->attr, NULL, NULL);
		free_handle(handle, 1);
	}
	else
	{
		//printf("\nNo rules are found!\n");
		throw_err(SYN_ERROR, UNK_EXPR);
		return false;
	}

	return true;
}

bool call_func_expr(Token *token, string *str)
{
	// precteme prvni argument
	get_token(token);
	if (token->type == R_PAR)
	{
		go_back(token);
		return true;
	}
	go_back(token);
	while (token->type != R_PAR)
	{
		arg_expr = true;
		if (!expression(token, NULL))
			return false;
		// vytvorime instrukce
		//create_instruction(INSTR_INSERT, &token->attr, NULL, NULL);
		create_instruction(INSTR_ASS_ARG, NULL, NULL, str);
		if (token->type != COMMA && token->type != R_PAR)
		{
			throw_err(SYN_ERROR, CALL_FUNC_ARG);
			return false;
		}
	}
	// vratime pravou zavorku zpet
	go_back(token);
	return true;
}

/*	Cteni dalsiho tokenu a priprava nove polozky 	*/
tItem *get_data(Token *token)
{   // vytvorime novou polozku a alokujeme novou pamet pro nej
	tItem *new_item;
	if ((new_item = init_item()) == NULL)
		return NULL;
    // cteme dalsi token ...
	if (!get_token(token) || token->type == T_EOF)
		return NULL;

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
bool shift_elem_to_term(tStack *src, tStack *dst)
{
	tItem *help;
	while ((src->stack[src->top])->i_type == EXPR || (src->stack[src->top])->i_type == DOWN_BOARD)
    {
		help = stack_pop(src);
		if (help == NULL || !stack_push(dst, help))
			return false;
	}
	return true;
}

/*	Funkce premisti vsechny elementy z jedneho zasobniku do jineho	*/
bool shift_all_elem(tStack *src, tStack *dst)
{
	tItem *help;
	while (!stack_empty(src))
    {
		help = stack_pop(src);
		if (help == NULL || !stack_push(dst, help))
			return false;
	}
	return true;
}

/*	Funkce inicializuje novou polozku seznamu */
tItem *init_item()
{
	tItem *new_item;
	if ((new_item = malloc(sizeof(tItem))) == NULL)
	{
		throw_err(ALLOC_ERROR, ALL_STRUCT);
		return NULL;
	}
	Token *new_token;
	if ((new_token = malloc(sizeof(Token))) == NULL)
	{
		throw_err(ALLOC_ERROR, ALL_STRUCT);
		return NULL;
	}
	strInit(&new_token->attr);
	new_item->data = new_token;
	return new_item;
}

/*	Funkce uvolni polozku staku	*/
void free_item(tItem *item)
{
	strFree(&item->data->attr);
	free(item->data);
	free(item);
}

/*	Funkce uvolni handle	*/
void free_handle(tItem **handle, int num)
{
	for (int i = 0; i < num; ++i)
    {
		if (handle[i]->data != NULL)
			free_token(handle[i]->data);

		free(handle[i]);
	}
}

/*	Inicializace zasobniku	*/
bool stack_init(tStack *st)
{
	st->top = -1;
	st->stack_size = DEF_STACK_SIZE;
	if ((st->stack = malloc(st->stack_size * sizeof(tItem *))) == NULL)
    {
		throw_err(ALLOC_ERROR, ALL_STRUCT);
		return false;
	}
	return true;
}

/*	Funkce uklada data na vrchol zasobniku	*/
bool stack_push(tStack *st, tItem *data)
{
	if (!stack_full(st))
    {
		st->top++;
		st->stack[st->top] = data;
	}
	else
    {
		st->stack_size += 1;
		if ((st->stack = realloc(st->stack, st->stack_size * sizeof(tItem *))) == NULL)
        {
			throw_err(ALLOC_ERROR, ALL_STRUCT);
			return false;
		}
		if (!stack_push(st, data))
			return false;
	}
	return true;
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
	free(main_st->stack);
	free(main_st);
	stack_destroy(help_st);
	free(help_st->stack);
	free(help_st);
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
				free(st->stack[i]->data);
			}
			free(st->stack[i]);
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
