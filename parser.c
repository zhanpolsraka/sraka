/* **************************************************************************/
/* Projekt:             Implementace interpretu jazyka IFJ16				*/
/* Predmet:             Formalni jazyky a prekladace (IFJ)					*/
/* Soubor:              parser.c  (Syntakticka analyza)						*/
/*																			*/
/* Autor login:      	Ermak Aleksei		xermak00						*/
/*                     	Khaitovich Anna		xkhait00						*/
/*						Nesmelova Antonina	xnesmel00						*/
/*						Fedorenko Oleg		xfedor00						*/
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

// indikace tridy 'Main'
bool main_flag = false;
bool in_main = false;
// indikace funkce 'run' v tele tridy 'Main'
bool run_flag = false;
// pomocna struktura pro zachovani informace o promenne/funkce
tHelpVar help;
// flag podminky
int condition = 0;
// typ navratove hodnoty
int ret_val_type = VOID;
bool returns = false;
// pocet argumentu
int arg = 0;
// pomocny ukazatel na uzel aktivni tridy a uzel aktivni funkci
tNode *class_node = NULL;
tNode *func_node = NULL;

/*	Zakladni funkce	*/
void parsing()
{
	Token *token = NULL;
	if ((token = malloc(sizeof(Token))) == NULL)
		throw_err(INT_ERROR, ALL_STRUCT, 0);
	mark_mem(token);
	strInit(&token->attr);
	strInit(&help.identifier);
	// syntakticka analyza
	class_list(token);
}

/* Zpracovani seznamu trid */
void class_list(Token *token)
{
	// cteme prvni token
	get_token(token);
	// konec programmu
	if (token->type == T_EOF && main_flag && run_flag)
        return;
	else if (token->type == T_EOF && !main_flag)
		throw_err(SEM_ERROR, UNDEF_CLASS, "Main");
	else if (token->type == T_EOF && !run_flag)
		throw_err(SEM_ERROR, UNDEF_FUNC, "run");
    // zpracuj tridu
	class(token);
    // pokracujeme rekurzivni zpracovani
	class_list(token);
}

/* Zpracovani statickych promennych uvnitr tridy */
void class(Token *token)
{
    // vyvola chybu pokud prectene tokeny neznamenaji zacatek tridy
	if (token->type != KEYWORD || strcmp(token->attr.str, "class"))
		throw_err(SYN_ERROR, EXPEC_TOKEN, "class");

    // potrebuje identifikator tridy
	get_token(token);
	if (token->type != IDENTIFIER)
		throw_err(SYN_ERROR, EXPEC_TOKEN, "identifier");

	//kontroluje neni-li dana trida "ifj16"
	if (equal_str(token->attr.str, "ifj16"))
		throw_err(SEM_ERROR, CLASS_EXIST, "ifj16");

	// kontroluje jestli trida se stejnym id uz existuje
	else if (get_node(&token->attr, CLASS, NULL) != NULL)
		throw_err(SEM_ERROR, CLASS_EXIST, token->attr.str);

	// vytvorime novou tridu v tabulce symbolu
	create_node(&token->attr, 0, CLASS, false);
	// zachovame ukazatel na aktivni tridu
	class_node = get_node(&token->attr, CLASS, NULL);
	// vytvorime instance tridy v instrukcnim zasobniku
	create_instance(&token->attr, INST_CLASS);

	// pokud trida ma nazev 'Main', oznacime ze trida Main existuje a program je v ni
	if (equal_str(token->attr.str, "Main"))
    {
		main_flag = true;
		in_main = true;
	}

	// potrebuje zacatek tela tridy (levou hranatou zavorku)
	get_token(token);
	if (token->type != L_VIN)
		throw_err(SYN_ERROR, EXPEC_TOKEN, "{");

	// zpracuje telo tridy
	class_body(token);
}

/* Zpracovani tela tridy */
void class_body(Token *token)
{
	get_token(token);
	// definice globalni promenne nebo globalni funkci
	if (token->type == KEYWORD && equal_str(token->attr.str, "static"))
    {
		get_token(token);
		// precetl datovy typ
		if (token->type >= INT && token->type <= BOOLEAN)
		{
			// zachovame informace o identifikatoru a typu
			help.st_static = true;
			help.type = token->type;
			// potrebujeme id promenne/funkci
			get_token(token);
			if (token->type != IDENTIFIER)
				throw_err(SYN_ERROR, EXPEC_TOKEN, "identifier");

			strCopyString(&help.identifier, &token->attr);

			// urcime jaky literal jde dal, na jehoz zaklade rozhodujeme o pristi akci
			get_token(token);
			if (token->type == L_PAR)
			    // jde leva kulata zavorka, volame zpracovani funkci
				function(token);

			else
				// jinak volame definici/inicializaci promenne
				define_var(token);

			// rekurzivne pokracujeme zpracovani tela tridy
			class_body(token);
		}
	}
	// precetl identifikator
	else if (token->type == IDENTIFIER)
	{
		strCopyString(&help.identifier, &token->attr);

		// inicializujeme promennu
		get_token(token);
		init_var(token);

		// rekurzivne pokracujeme zpracovani tela tridy
		class_body(token);
	}
	// konec tela tridy
	else if (token->type == R_VIN)
	{
		create_instance(NULL, INST_END_CLASS);
		// pokud vyjdeme z Mainu zrusime flag
		if(in_main)
			in_main = false;
	}
	else
		throw_err(SYN_ERROR, UNK_EXPR, token->attr.str);
}

/* Zparcovani funkci */
void function(Token *token)
{
    // zavola chybu pokud funkce se stejnym id uz existuje
    if (get_node(&help.identifier, FUNCTION, class_node->functions) != NULL)
		throw_err(SEM_ERROR, FUNC_EXIST, help.identifier.str);

    // detekujeme funkci "run" v tride "Main"
    if (equal_str(help.identifier.str, "run"))
    {   // kontrolujeme aby typ navratove hodnoty byl void
        if (in_main)
        {
            if (help.type != VOID)
				throw_err(SEM_TYPE_ERROR, TYPE_KEY, 0);

            // nastavime flag
            run_flag = true;
        }
    }
    // vytvorime novou funkci v tabulce symbolu
    create_node(&help.identifier, help.type, FUNCTION, true);
	// zachovame ukazatel na uzel aktivni funkci
	func_node = get_node(&help.identifier, FUNCTION, class_node->functions);
	// vytvorime novou instance v instrukcnim listu
	create_instance(&help.identifier, INST_FUNCTION);
	ret_val_type = help.type;
    // zpracujeme seznam argumentu funkci
    argument_list(token);
	// precteme levou hranatou zavorku (zacatek bloku)
	get_token(token);
	if (token->type != L_VIN)
		throw_err(SYN_ERROR, EXPEC_TOKEN, "{");

	// zpracujeme telo funkci
    statement_list(token);
}

/* Zpracovani seznamu argumentu */
void argument_list(Token *token)
{
	get_token(token);
	// konec seznamu argumentu
	if (token->type == R_PAR)
        return;

	// potrebujeme typ argumentu
	else if (token->type >= INT && token->type <= BOOLEAN)
    {
		help.type = token->type;
        // potrebujeme id argumentu
		get_token(token);
		if (token->type != IDENTIFIER)
			throw_err(SYN_ERROR, EXPEC_TOKEN, "identifier");

		// vytvorime novy argument funkci v tabulce symbolu
		create_arg(&token->attr, help.type);
		// pokracujeme zpracovani seznamu argumentu
		get_token(token);
		next_arg(token);
	}
	else
		throw_err(SYN_ERROR, UNK_EXPR, token->attr.str);
}

void next_arg(Token *token)
{
	if (token->type == COMMA)
    {
		get_token(token);

		if (token->type >= INT && token->type <= BOOLEAN)
		{
			help.type = token->type;
			get_token(token);
			if (token->type != IDENTIFIER)
				throw_err(SYN_ERROR, EXPEC_TOKEN, "identifier");

			// vytvorime novy argument funkci v tabulce symbolu
			create_arg(&token->attr, help.type);
			// pokracujeme rekurzivni zpracovani seznamu argumentu
			get_token(token);
			next_arg(token);
		}
		else
			throw_err(SYN_ERROR, UNK_EXPR, token->attr.str);
	}
	// konec seznamu argumentu
	else if (token->type == R_PAR)
        return;
	else
		throw_err(SYN_ERROR, UNK_EXPR, token->attr.str);
}

/* Zpracovani seznamu argumentu volane funkci */
void call_arg_list(Token *token)
{
	get_token(token);
	// konec seznamu argumentu
	if (token->type == R_PAR)
	{
		if (equal_str(help.identifier.str, "ifj16.print") ||
			equal_str(help.identifier.str, "ifj16.length") ||
			equal_str(help.identifier.str, "ifj16.substr") ||
			equal_str(help.identifier.str, "ifj16.compare") ||
			equal_str(help.identifier.str, "ifj16.find") ||
			equal_str(help.identifier.str, "ifj16.sort"))
			throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG, help.identifier.str);
		else
		{
			// vytvorime instrukce CREATE_FRAME pokud volana funkce neni vestavena
			if (!equal_str(help.identifier.str, "ifj16.print") &&
				!equal_str(help.identifier.str, "ifj16.length") &&
				!equal_str(help.identifier.str, "ifj16.substr") &&
				!equal_str(help.identifier.str, "ifj16.compare") &&
				!equal_str(help.identifier.str, "ifj16.find") &&
				!equal_str(help.identifier.str, "ifj16.sort"))
				create_instruction(CREATE_FRAME, &help.identifier, NULL, NULL);
			return;
		}
	}
	else
		go_back(token);

	arg_expr++;
	expression(token, NULL);
	if (!equal_str(help.identifier.str, "ifj16.print"))
		arg++;
	// pokracujeme zpracovani seznamu argumentu
	if (!equal_str(help.identifier.str, "ifj16.print"))
		call_next_arg(token);
}

void call_next_arg(Token *token)
{
	if (token->type == COMMA)
    {
		// instrukce
		arg_expr++;
		expression(token, NULL);
		arg++;
		//create_instruction(INSTR_ASS_ARG, NULL, NULL, NULL);
		// pokracujeme zpracovani seznamu argumentu
		call_next_arg(token);
	}
	// konec seznamu argumentu
	else if (token->type == R_PAR)
	{
		// vytvorime instrukce CREATE_FRAME pokud volana funkce neni vestavena
		if (!equal_str(help.identifier.str, "ifj16.print") &&
			!equal_str(help.identifier.str, "ifj16.length") &&
			!equal_str(help.identifier.str, "ifj16.substr") &&
			!equal_str(help.identifier.str, "ifj16.compare") &&
			!equal_str(help.identifier.str, "ifj16.find") &&
			!equal_str(help.identifier.str, "ifj16.sort"))
		{
			create_instruction(CREATE_FRAME, &help.identifier, NULL, NULL);
			// priradime argumentum hodnoty
			for (; arg != 0; arg--)
				create_instruction(INSTR_ASS_ARG, NULL, NULL, NULL);
		}
		// zkontrolujeme pocet argumentu
		else if (
			(equal_str(help.identifier.str, "ifj16.length") && arg != 1) ||
			(equal_str(help.identifier.str, "ifj16.sort") && arg != 1) ||
			(equal_str(help.identifier.str, "ifj16.substr") && arg != 3) ||
			(equal_str(help.identifier.str, "ifj16.compare") && arg != 2) ||
			(equal_str(help.identifier.str, "ifj16.find") && arg != 2))
			throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG, help.identifier.str);
		arg = 0;
		return;
	}
	else
		throw_err(SYN_ERROR, UNK_EXPR, token->attr.str);
}

/* Zpracovani tela funkci */
void statement_list(Token *token)
{
	get_token(token);
	// definice lokalni promenne
	if (token->type >= INT && token->type <= BOOLEAN && !condition)
    {   // zachovame typ promenne
		help.type = token->type;
        // potrebujeme id promenne
		get_token(token);
		if (token->type != IDENTIFIER)
			throw_err(SYN_ERROR, EXPEC_TOKEN, "identifier");
		strCopyString(&help.identifier, &token->attr);
		help.st_static = false;

		// precteme pristi znak
		get_token(token);
        // definujeme promennu
		define_var(token);
	}
	// inicializace promenne nebo volani funkci
	else if (token->type == IDENTIFIER)
    {
		strCopyString(&help.identifier, &token->attr);
		// kontrolujeme otevrenou zavorku pokud jde o volani funkci
		get_token(token);
		if (token->type == L_PAR)
        {
			// zpracujeme argumenty volane funkci
			call_arg_list(token);
            // precteme strednik
			get_token(token);
			// zavolame funkci
			if (equal_str(help.identifier.str, "ifj16.print"))
				create_instruction(INSTR_PRINT, NULL, NULL, NULL);
			else if (equal_str(help.identifier.str, "ifj16.length"))
				create_instruction(INSTR_LENGTH, NULL, NULL, NULL);
			else if (equal_str(help.identifier.str, "ifj16.substr"))
				create_instruction(INSTR_SUBSTR, NULL, NULL, NULL);
			else if (equal_str(help.identifier.str, "ifj16.compare"))
				create_instruction(INSTR_COMPARE, NULL, NULL, NULL);
			else if (equal_str(help.identifier.str, "ifj16.find"))
				create_instruction(INSTR_FIND, NULL, NULL, NULL);
			else if (equal_str(help.identifier.str, "ifj16.sort"))
				create_instruction(INSTR_SORT, NULL, NULL, NULL);
			else
				create_instruction(INSTR_CALL_FUNC, NULL, NULL, &help.identifier);
		}
		else
		{	// nejde o volani funkci
            // inicializujeme promennu
    		init_var(token);
		}
	}
	// volame zpracovani ridici struktury
	else if (token->type == KEYWORD)
    {
        ride_struct(token);
    }
	// konec bloku
	else if (token->type == R_VIN)
	{
		if (!condition)
		{
			if (ret_val_type != VOID && !returns)
				throw_err(UNINIT_ERROR, RET_VALUE, 0);
			create_instance(NULL, INST_END_FUNCTION);
			ret_val_type = VOID;
			returns = false;
		}
		else
			condition--;
		return;
	}
	// chyba
	else
		throw_err(SYN_ERROR, UNK_EXPR, token->attr.str);

	// pokracujeme rekurzivni zpracovani vyrazu
	statement_list(token);
}

void ride_struct(Token *token)
{
    if (equal_str(token->attr.str, "if"))
    {
		create_instruction(INSTR_BEG_COND, NULL, NULL, NULL);
		get_token(token);
		if (token->type == L_PAR)
		{
			// pro zpracovani vyrazu precedencni tabulkou
			// vratime levou zavorku zpet
			go_back(token);
			cond_expr = true;
			expression(token, NULL);
			// vytvarime instrukce
			create_instruction(INSTR_IF, NULL, NULL, NULL);
			// zpracujeme blok
			condition++;
			statement_list(token);
			create_instruction(INSTR_END_BLCK, NULL, NULL, NULL);

			get_token(token);
			if (equal_str(token->attr.str, "else"))
			{
				// vytvarime instrukce
				create_instruction(INSTR_ELSE, NULL, NULL, NULL);
				// precteme levou hranatou zavorku (zacatek bloku)
				get_token(token);
				if (token->type != L_VIN)
					throw_err(SYN_ERROR, EXPEC_TOKEN, "{");
				// zpracujeme blok
				condition++;
				statement_list(token);
				create_instruction(INSTR_END_BLCK, NULL, NULL, NULL);
			}
			else
				go_back(token);
		}
		else
			throw_err(SYN_ERROR, UNK_EXPR, token->attr.str);
    }
    else if (equal_str(token->attr.str, "while"))
    {
		create_instruction(INSTR_BEG_COND, NULL, NULL, NULL);
		get_token(token);
		if (token->type == L_PAR)
		{
			// pro zpracovani vyrazu precedencni tabulkou
			// vratime levou zavorku zpet
			go_back(token);
			cond_expr = true;
	        expression(token, NULL);
			// vytvarime instrukce
			create_instruction(INSTR_WHILE, NULL, NULL, NULL);
			// zpracujeme blok
			condition++;
	        statement_list(token);
			create_instruction(INSTR_END_BLCK, NULL, NULL, NULL);
		}
		else
			throw_err(SYN_ERROR, UNK_EXPR, token->attr.str);
    }
    else if (equal_str(token->attr.str, "return"))
    {
		get_token(token);
		if ((token->type != SEMICOLON && ret_val_type == VOID) ||
			(token->type == SEMICOLON && ret_val_type != VOID))
			throw_err(UNINIT_ERROR, RET_VALUE, 0);
		go_back(token);
		returns = true;
        expression(token, NULL);
		create_instruction(INSTR_RETURN, NULL, NULL, NULL);
    }
	else if (equal_str(token->attr.str, "break"))
	{
		create_instruction(INSTR_BREAK, NULL, NULL, NULL);
		// precteme strednik
		get_token(token);
	}
	else if (equal_str(token->attr.str, "continue"))
	{
		create_instruction(INSTR_CONTINUE, NULL, NULL, NULL);
		// precteme strednik
		get_token(token);
	}
    else
		throw_err(SYN_ERROR, UNK_EXPR, token->attr.str);
}

/* Definice promenne */
void define_var(Token *token)
{
	// zkontrolujeme na pritomnost promenne se stejnym nazvem ve funkci/tride
	if (!help.st_static)
	{
		if (get_node(&help.identifier, VARIABLE, func_node->variables) != NULL)
			throw_err(SEM_ERROR, VAR_EXIST, help.identifier.str);
	}
	else
	{
		if (get_node(&help.identifier, VARIABLE, class_node->variables) != NULL)
			throw_err(SEM_ERROR, VAR_EXIST, help.identifier.str);
	}
    // vytvorime novou promennou v tabulce symbolu
    create_node(&help.identifier, help.type, VARIABLE, help.st_static);
	// vytvorime instrukce definici
	create_instruction(DEFINE_VAR, &help.identifier, NULL, NULL);
    // pokud dale jde strednik -> vyjdeme
    if (token->type == SEMICOLON)
		return;
    // jinak muze jit dale jenom vyraz
    else if (token->type == ASSIGNMENT)
    {
		init_var(token);
    }
	else
		throw_err(SYN_ERROR, UNK_EXPR, token->attr.str);
}

/* Inicializace promenne */
void init_var(Token *token)
{
    if (token->type == ASSIGNMENT)
    {
    	expression(token, &help.identifier);
    }
	else
		throw_err(SYN_ERROR, UNK_EXPR, token->attr.str);
}
