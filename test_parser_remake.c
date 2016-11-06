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

#define PARSE_SUCCESSFUL 1
#define PARSE_UNSUCCESSFUL 0

// indikace tridy 'Main'
bool main_flag = false;
bool in_main = false;
// indikace funkce 'run' v tele tridy 'Main'
bool run_flag = false;
// pomocna struktura pro zachovani informace o promenne/funkce
tHelpVar help;
// flag podminky
bool condition = false;
// flag volani funkci
bool call_func = false;

// test
int parsing_succesful()
{
	bool error = false;
	Token *token = NULL;
	if ((token = malloc(sizeof(Token))) == NULL)
	{
		throw_err(ALLOC_ERROR, ALL_STRUCT);
		return false;
	}
	strInit(&token->attr);

	strInit(&help.identifier);

	// syntakticka analyza
	if (class_list(token))
		error = false;
	else
		error = true;

	strFree(&help.identifier);
	free_token(token);

	if (error)
		return PARSE_UNSUCCESSFUL;
	else
		return PARSE_SUCCESSFUL;
}

// zpracovani celeho programmu
bool class_list(Token *token)
{	// konec programmu
	if (get_token(token) && token->type == T_EOF && main_flag && run_flag)
        return true;
    // jinak zpracuj tridu
    if (!class(token))
        return false;
    // pokracujeme zpracovani
    if (!class_list(token))
        return false;

    return true;
}

bool class(Token *token)
{
    // vyvola chybu pokud prectene tokeny neznamenaji zacatek tridy
	if (token->type != KEYWORD || strcmp(token->attr.str, "class"))
    {
		throw_err(UNK_CLASS_BEG, CLASS_KEY);
		return false;
	}
    // potrebuje identifikator tridy
	else if (!get_token(token) || token->type != IDENTIFIER)
    {
		throw_err(SYN_ERROR, ID_KEY);
		return false;
	}
	// kontroluje jestli trida se stejnym id uz existuje
	else if (get_node(&token->attr, CLASS, NULL) != NULL)
    {
		throw_err(CLASS_EXIST, TREE_CLASS);
		return false;
	}
	// vytvorime novou tridu v tabulce symbolu
	if (!create_node(&token->attr, CLASS, 0, false))
		return false;

	if (!create_instance(&token->attr, INST_CLASS))
		return false;

	// pokud trida ma nazev 'Main', oznacime ze trida Main existuje a program je v ni
	if (!strcmp(token->attr.str, "Main"))
    {
		main_flag = true;
		in_main = true;
	}
	// zpracujeme telo tridy
	if (!get_token(token) || token->type != L_VIN || !class_body(token))
		return false;

	return true;
}

bool class_body(Token *token)
{
	if (!get_token(token))
		return false;
	// definice globalni promenne nebo globalni funkci
	else if (token->type == KEYWORD && !strcmp(token->attr.str, "static") &&
			get_token(token) && token->type >= INT && token->type <= BOOLEAN)
    {
        // zachovame informace o identifikatoru a typu
		help.st_static = true;
		help.type = token->type;
        // potrebujeme id promenne/funkci
		if (!get_token(token) || token->type != IDENTIFIER)
        {
			throw_err(SYN_ERROR, ID_KEY);
			return false;
		}
        strCopyString(&help.identifier, &token->attr);

        // urcime jaky literal jde dal, na jehoz zaklade rozhodujeme o pristi akci
        if (get_token(token) && token->type == L_PAR)
        {   // jde leva kulata zavorka, volame zpracovani funkci
            if (!function(token))
                return false;
        }
        else
        {
            // jinak volame definici/inicializaci promenne
            if (!define_var(token))
            	return false;
        }

        // rekurzivne pokracujeme zpracovani tela tridy
		if (!class_body(token))
			return false;
	}
	else if (token->type == IDENTIFIER)
	{
		strCopyString(&help.identifier, &token->attr);

		// inicializujeme promennu
    	if (!get_token(token) || !init_var(token))
    		return false;

		// rekurzivne pokracujeme zpracovani tela tridy
		if (!class_body(token))
			return false;
	}
	// konec tela tridy
	else if (token->type == R_VIN)
	{
		if (!create_instance(NULL, INST_END_CLASS))
			return false;
		// pokud vyjdeme z Mainu zrusime flag
		if(in_main) in_main = false;
		return true;
	}
	else
    {
		throw_err(SYN_ERROR, UNK_EXPR);
		return false;
	}

	return true;
}

bool function(Token *token)
{
    // zavola chybu pokud funkce se stejnym id uz existuje
    if (get_node(&help.identifier, FUNCTION, NULL) != NULL)
    {
        throw_err(FUNC_EXIST, TREE_FUNC);
        return false;
    }
    // detekujeme funkci "run" v tride "Main"
    if (!strcmp(help.identifier.str, "run"))
    {   // kontrolujeme aby typ navratove hodnoty byl void
        if (in_main)
        {
            if (help.type != VOID)
            {
                throw_err(SEM_ERROR, TYPE_KEY);
                return false;
            }
            // nastavime flag
            run_flag = true;
        }
    }
    // vytvorime novou funkci v tabulce symbolu
    if (!create_node(&help.identifier, FUNCTION, help.type, true))
        return false;
	// vytvorime novou instance v instrukcnim listu
	if (!create_instance(&help.identifier, INST_FUNCTION))
		return false;
    // zpracujeme seznam argumentu funkci
    if (!argument_list(token))
        return false;
	// precteme levou hranatou zavorku (zacatek bloku)
	if (!get_token(token) || token->type != L_VIN)
	    return false;
    // zpracujeme telo funkci
    if (!statement_list(token))
            return false;

    return true;
}

bool argument_list(Token *token)
{
	// konec seznamu argumentu
	if (get_token(token) && token->type == R_PAR)
        return true;
    // potrebujeme typ argumentu
	else if (token->type >= INT && token->type <= BOOLEAN)
    {
		help.type = token->type;
        // potrebujeme id argumentu
		if (!get_token(token) || token->type != IDENTIFIER)
        {
			throw_err(SYN_ERROR, ID_KEY);
			return false;
		}
		// vytvorime novy argument funkci v tabulce symbolu
		if (!create_arg(&token->attr, help.type))
			return false;
		// pokracujeme zpracovani seznamu argumentu
		if (!next_arg(token))
			return false;
	}
	// zpracovani argumentu jestli funkce je volana
    else if (call_func && (token->type == VALUE || token->type == IDENTIFIER))
    {
    	// instrukce
		create_instruction(INSTR_INSERT, &token->attr, NULL, NULL);
		create_instruction(INSTR_ASS_ARG,  NULL, NULL, &help.identifier);
		// pokracujeme zpracovani seznamu argumentu
		if (!next_arg(token))
			return false;
    }
	else
    {
		throw_err(SYN_ERROR, UNK_EXPR);
		return false;
	}

	return true;
}

bool next_arg(Token *token)
{
    // konec seznamu argumentu
	if (get_token(token) && token->type == R_PAR)
        return true;

	else if (token->type == COMMA)
    {
		if (get_token(token) && token->type >= INT && token->type <= BOOLEAN)
		{
			help.type = token->type;
			if (!get_token(token) || token->type != IDENTIFIER)
			{
				throw_err(SYN_ERROR, ID_KEY);
				return false;
			}
			// vytvorime novy argument funkci v tabulce symbolu
			if (!create_arg(&token->attr, help.type))
			return false;
			// pokracujeme rekurzivni zpracovani seznamu argumentu
			if (!next_arg(token))
			return false;
		}
		else if (call_func && (token->type == VALUE || token->type == IDENTIFIER))
		{
			// instrukce
			create_instruction(INSTR_INSERT, &token->attr, NULL, NULL);
			create_instruction(INSTR_ASS_ARG, NULL, NULL, &help.identifier);
			// pokracujeme zpracovani seznamu argumentu
			if (!next_arg(token))
			return false;
		}
		else
	    {
			throw_err(SYN_ERROR, UNK_EXPR);
			return false;
		}
	}
	else
    {
		throw_err(SYN_ERROR, UNK_EXPR);
		return false;
	}
	return true;
}

bool statement_list(Token *token)
{
	if(!get_token(token))
		return false;
	// definice lokalni promenne
	if (token->type >= INT && token->type <= BOOLEAN && !condition)
    {   // zachovame typ promenne
		help.type = token->type;
        // potrebujeme id promenne
		if (!get_token(token) || token->type != IDENTIFIER)
        {
			throw_err(SYN_ERROR, ID_KEY);
			return false;
		}
		strCopyString(&help.identifier, &token->attr);
		help.st_static = false;

		// precteme znak "ravno" a
        // definujeme promennu
		if (!get_token(token) || !define_var(token))
			return false;
	}
	// inicializace promenne nebo volani funkci
	else if (token->type == IDENTIFIER)
    {
		strCopyString(&help.identifier, &token->attr);
		// kontrolujeme otevrenou zavorku pokud jde o volani funkci
		if (get_token(token) && token->type == L_PAR)
        {   // zpracujeme argumenty volane funkci
            call_func = true;
			if (!argument_list(token))
				return false;
            call_func = false;
            // precteme strednik
            if (!get_token(token))
                return false;
			// zavolame funkci
			create_instruction(INSTR_CALL_FUNC, NULL, NULL, &help.identifier);
		}
		else
		{	// nejde o volani funkci
            // inicializujeme promennu
    		if (!init_var(token))
    			return false;
		}
	}
	// volame zpracovani ridici struktury
	else if (token->type == KEYWORD)
    {
        if (!ride_struct(token))
            return false;
    }
	// konec bloku
	else if (token->type == R_VIN)
	{
		if (!condition)
		{
			if (!create_instance(NULL, INST_END_FUNCTION))
			return false;
		}
		return true;
	}
	// chyba
	else
    {
		throw_err(SYN_ERROR, UNK_EXPR);
		return false;
	}

	// pokracujeme rekurzivni zpracovani vyrazu
	if (!statement_list(token))
		return false;
	else
		return true;
}

bool ride_struct(Token *token)
{
	create_instruction(INSTR_BEG_COND, NULL, NULL, NULL);

    if (!strcmp(token->attr.str, "if") &&
        get_token(token) && token->type == L_PAR)
    {
		// pro zpracovani vyrazu precedencni tabulkou
		// vratime levou zavorku zpet
		go_back(token);
		cond_expr = true;
        if (expression(token, NULL))
        {
			// vytvarime instrukce
			create_instruction(INSTR_IF, NULL, NULL, NULL);
			// zpracujeme blok
			condition = true;
            if (!statement_list(token))
                return false;
			condition = false;
			create_instruction(INSTR_END_BLCK, NULL, NULL, NULL);
        }
        else
        {
            throw_err(SYN_ERROR, UNK_EXPR);
            return false;
        }

        if (get_token(token) && !strcmp(token->attr.str, "else"))
        {
			// vytvarime instrukce
			create_instruction(INSTR_ELSE, NULL, NULL, NULL);
			// precteme levou hranatou zavorku (zacatek bloku)
			if (!get_token(token) || token->type != L_VIN)
			    return false;
			// zpracujeme blok
			condition = true;
            if(!statement_list(token))
                return false;
			condition = false;
			create_instruction(INSTR_END_BLCK, NULL, NULL, NULL);
        }
        else
            go_back(token);
    }
    else if (!strcmp(token->attr.str, "while") &&
			get_token(token) && token->type == L_PAR)
    {
		// pro zpracovani vyrazu precedencni tabulkou
		// vratime levou zavorku zpet
		go_back(token);
		cond_expr = true;
        if (expression(token, NULL))
        {
			// vytvarime instrukce
			create_instruction(INSTR_WHILE, NULL, NULL, NULL);
			condition = true;
			// zpracujeme blok
            if(!statement_list(token))
                return false;
			condition = false;
			create_instruction(INSTR_END_BLCK, NULL, NULL, NULL);
        }
        else
        {
            throw_err(SYN_ERROR, UNK_EXPR);
            return false;
        }
    }
    else if (!strcmp(token->attr.str, "return"))
    {
        if (!expression(token, NULL))
			return false;
    }
    else
    {
        throw_err(SYN_ERROR, UNK_EXPR);
        return false;
    }

    return true;
}

bool define_var(Token *token)
{
    // vytvorime novou promennou v tabulce symbolu
    if (!create_node(&help.identifier, VARIABLE, help.type, help.st_static))
    	return false;
    // strednik
    if (token->type == SEMICOLON)
    	return true;
    // vyraz
    else if (token->type == ASSIGNMENT)
    {
    	if (!init_var(token))
    		return false;
    }
	else
	{
    	throw_err(SYN_ERROR, UNK_EXPR);
    	return false;
    }

    return true;
}

bool init_var(Token *token)
{
    if (token->type == ASSIGNMENT)
    {
    	if (!expression(token, &help.identifier))
    		return false;
    }
	else
	{
    	throw_err(SYN_ERROR, UNK_EXPR);
    	return false;
    }

	return true;
}
