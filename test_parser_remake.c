#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "str.h"
#include "test_inst.h"
#include "test_scanner.h"
#include "test_precedence_remake.h"
#include "test_table.h"
#include "test_parser_remake.h"
#include "test_error.h"
//#include "test_interpret.h"

// indikace tridy 'Main'
bool main_flag = false;
bool in_main = false;
// indikace funkce 'run' v tele tridy 'Main'
bool run_flag = false;
// pomocna struktura pro zachovani informace o promenne/funkce
tHelpVar help;
// flag podminky
int condition = 0;

/*	Zakladni funkce	*/
void parsing()
{
	Token *token = NULL;
	if ((token = malloc(sizeof(Token))) == NULL)
		throw_err(ALLOC_ERROR, ALL_STRUCT);

	strInit(&token->attr);

	strInit(&help.identifier);

	// syntakticka analyza
	class_list(token);

	strFree(&help.identifier);
	free_token(token);
}

// zpracovani celeho programmu
void class_list(Token *token)
{
	// cteme prvni token
	get_token(token);
	// konec programmu
	if (token->type == T_EOF && main_flag && run_flag)
        return;
    // dostali jsme slovo "class" -> zpracuj tridu
	class(token);
    // pokracujeme rekurzivni zpracovani
	class_list(token);
}

void class(Token *token)
{
    // vyvola chybu pokud prectene tokeny neznamenaji zacatek tridy
	if (token->type != KEYWORD || strcmp(token->attr.str, "class"))
	{
		printf("----> parser, class\n");
		throw_err(UNK_CLASS_BEG, CLASS_KEY);
	}

    // potrebuje identifikator tridy
	get_token(token);
	if (token->type != IDENTIFIER)
	{
		printf("----> parser, class\n");
		throw_err(SYN_ERROR, ID_KEY);
	}

	// kontroluje jestli trida se stejnym id uz existuje
	else if (get_node(&token->attr, CLASS, NULL) != NULL)
	{
		printf("----> parser, class\n");
		throw_err(CLASS_EXIST, TREE_CLASS);
	}

	// vytvorime novou tridu v tabulce symbolu
	create_node(&token->attr, CLASS, 0, false);

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
	{
		printf("----> parser, class\n");
		throw_err(SYN_ERROR, UNK_EXPR);
	}

	// zpracuje telo tridy
	class_body(token);
}

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
			{
				printf("----> parser, class_body\n");
				throw_err(SYN_ERROR, ID_KEY);
			}

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
	{
		printf("----> parser, class_body\n");
		throw_err(SYN_ERROR, UNK_EXPR);
	}
}

void function(Token *token)
{
    // zavola chybu pokud funkce se stejnym id uz existuje
    if (get_node(&help.identifier, FUNCTION, NULL) != NULL)
	{
		printf("----> parser, function\n");
		throw_err(FUNC_EXIST, TREE_FUNC);
	}

    // detekujeme funkci "run" v tride "Main"
    if (equal_str(help.identifier.str, "run"))
    {   // kontrolujeme aby typ navratove hodnoty byl void
        if (in_main)
        {
            if (help.type != VOID)
			{
				printf("----> parser, function\n");
				throw_err(SEM_ERROR, TYPE_KEY);
			}

            // nastavime flag
            run_flag = true;
        }
    }
    // vytvorime novou funkci v tabulce symbolu
    create_node(&help.identifier, FUNCTION, help.type, true);
	// vytvorime novou instance v instrukcnim listu
	create_instance(&help.identifier, INST_FUNCTION);
    // zpracujeme seznam argumentu funkci
    argument_list(token);
	// precteme levou hranatou zavorku (zacatek bloku)
	get_token(token);
	if (token->type != L_VIN)
	{
		printf("----> parser, function\n");
		throw_err(SYN_ERROR, UNK_EXPR);
	}

	// zpracujeme telo funkci
    statement_list(token);
}

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
		{
			printf("----> parser, argument_list\n");
			throw_err(SYN_ERROR, ID_KEY);
		}

		// vytvorime novy argument funkci v tabulce symbolu
		create_arg(&token->attr, help.type);
		// pokracujeme zpracovani seznamu argumentu
		get_token(token);
		next_arg(token);
	}
	else
	{
		printf("----> parser, argument_list\n");
		throw_err(SYN_ERROR, UNK_EXPR);
	}
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
			{
				printf("----> parser, next_arg\n");
				throw_err(SYN_ERROR, ID_KEY);
			}

			// vytvorime novy argument funkci v tabulce symbolu
			create_arg(&token->attr, help.type);
			// pokracujeme rekurzivni zpracovani seznamu argumentu
			get_token(token);
			next_arg(token);
		}
		else
		{
			printf("----> parser, next_arg\n");
			throw_err(SYN_ERROR, UNK_EXPR);
		}
	}
	// konec seznamu argumentu
	else if (token->type == R_PAR)
        return;
	else
	{
		printf("----> parser, next_arg\n");
		throw_err(SYN_ERROR, UNK_EXPR);
	}
}

void call_arg_list(Token *token)
{
	get_token(token);
	// konec seznamu argumentu
	if (token->type == R_PAR)
	{
		if (equal_str(help.identifier.str, "ifj16.print"))
		{
			printf("----> parser, call_arg_list\n");
			throw_err(SEM_ERROR, CALL_FUNC_ARG);
		}
		else
			return;
	}
	else
		go_back(token);

	arg_expr++;
	expression(token, NULL);
	if (!equal_str(help.identifier.str, "ifj16.print"))
		{
			create_instruction(INSTR_ASS_ARG,  NULL, NULL, &help.identifier);
		}
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
		create_instruction(INSTR_ASS_ARG, NULL, NULL, &help.identifier);
		// pokracujeme zpracovani seznamu argumentu
		next_arg(token);
	}
	// konec seznamu argumentu
	else if (token->type == R_PAR)
        return;
	else
	{
		printf("----> parser, call_next_list\n");
		throw_err(SYN_ERROR, UNK_EXPR);
	}
}

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
		{
			printf("----> parser, statement_list\n");
			throw_err(SYN_ERROR, ID_KEY);
		}

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
			create_instance(NULL, INST_END_FUNCTION);
		else
			condition--;
		return;
	}
	// chyba
	else
	{
		printf("----> parser, statement_list\n");
		throw_err(SYN_ERROR, UNK_EXPR);
	}

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
				{
					printf("----> parser, ride_struct\n");
					throw_err(SYN_ERROR, UNK_EXPR);
				}
				// zpracujeme blok
				condition++;
				statement_list(token);
				create_instruction(INSTR_END_BLCK, NULL, NULL, NULL);
			}
			else
				go_back(token);
		}
		else
		{
			printf("----> parser, ride_struct\n");
			throw_err(SYN_ERROR, UNK_EXPR);
		}
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
			condition = true;
			// zpracujeme blok
	        statement_list(token);
			condition = false;
			create_instruction(INSTR_END_BLCK, NULL, NULL, NULL);
		}
		else
		{
			printf("----> parser, ride_struct\n");
			throw_err(SYN_ERROR, UNK_EXPR);
		}
    }
    else if (equal_str(token->attr.str, "return"))
    {
        expression(token, NULL);
		//create_instruction(INSTR_RETURN, NULL, NULL, NULL);
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
	{
		printf("----> parser, ride_struct\n");
		throw_err(SYN_ERROR, UNK_EXPR);
	}
}

void define_var(Token *token)
{
    // vytvorime novou promennou v tabulce symbolu
    create_node(&help.identifier, VARIABLE, help.type, help.st_static);
    // strednik
    if (token->type == SEMICOLON)
		return;
    // vyraz
    else if (token->type == ASSIGNMENT)
    {
		init_var(token);
    }
	else
	{
		printf("----> parser, define_var\n");
		throw_err(SYN_ERROR, UNK_EXPR);
	}
}

void init_var(Token *token)
{
    if (token->type == ASSIGNMENT)
    {
    	expression(token, &help.identifier);
    }
	else
	{
		printf("----> parser, init_var\n");
		throw_err(SYN_ERROR, UNK_EXPR);
	}
}
