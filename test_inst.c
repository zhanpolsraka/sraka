#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "str.h"
#include "test_scanner.h"
#include "test_parser_remake.h"
#include "test_error.h"
#include "test_inst.h"
#include "test_table.h"

tInstrStack *main_instr_stack;
tNode *current_arg;

// создаеь инстанции в стэке
bool create_instance(string *name, int type)
{
    tInstance *new_instance;
    if ((new_instance = malloc(sizeof(tInstance))) == NULL)
    {
        throw_err(ALLOC_ERROR, ALL_STRUCT);
        return false;
    }
    if (name != NULL)
    {
        if ((new_instance->name = malloc(sizeof(string))) == NULL ||
        strInit(new_instance->name))
        {
            throw_err(ALLOC_ERROR, ALL_STRUCT);
            return false;
        }
        strCopyString(new_instance->name, name);
    }
    // сохраняет инфу
    new_instance->instr = NULL;
    new_instance->type = type;
    // кладет в стэк
    if (!instr_stack_push(main_instr_stack, new_instance))
        return false;
    return true;
}

// создает инструкции
bool create_instruction(int op, void *addr1, void *addr2, void *addr3)
{
    tInstruction *new_instr;

    if ((new_instr = malloc(sizeof(tInstruction))) == NULL)
    {
        throw_err(ALLOC_ERROR, ALL_STRUCT);
        return false;
    }
    new_instr->op = op;

    // записывает строки в адресса операндов
    // создаст новую строку для названия каждого операнда
    if (addr1 != NULL)
    {
        string *addr1_name = NULL;
        if ((addr1_name = malloc(sizeof(string))) == NULL)
        {
            throw_err(ALLOC_ERROR, ALL_STRUCT);
            return false;
        }
        strInit(addr1_name);
        strCopyString(addr1_name, addr1);
        new_instr->addr1 = addr1_name;
    }
    if (addr2 != NULL)
    {
        string *addr2_name = NULL;
        if ((addr2_name = malloc(sizeof(string))) == NULL)
        {
            throw_err(ALLOC_ERROR, ALL_STRUCT);
            return false;
        }
        strInit(addr2_name);
        strCopyString(addr2_name, addr1);
        new_instr->addr1 = addr2_name;
    }
    if (addr3 != NULL)
    {
        string *addr3_name = NULL;
        if ((addr3_name = malloc(sizeof(string))) == NULL)
        {
            throw_err(ALLOC_ERROR, ALL_STRUCT);
            return false;
        }
        strInit(addr3_name);
        strCopyString(addr3_name, addr3);
        new_instr->addr3 = addr3_name;
    }

    tInstance *new_inst;
    if ((new_inst = malloc(sizeof(tInstance))) == NULL)
    {
        throw_err(ALLOC_ERROR, ALL_STRUCT);
        return false;
    }
    new_inst->type = INST_INSTRUCTION;
    new_inst->instr = new_instr;

    if (!instr_stack_push(main_instr_stack, new_inst))
        return false;

    return true;
}

void *search_var(string *str, tNode *begin, int type);
tData *make_data(string *str);

// создаст связи в таблице символов для определенных инструкций
bool make_relations(tInstrStack *st)
{
    tNode *begin;
    tNode *curr_class;

    for (int i = st->top; i > -1; i--)
    {
        if (st->inst[i]->type == INST_CLASS)
        {
            begin = get_node(st->inst[i]->name, CLASS, NULL);
            curr_class = begin;
        }
        else if (st->inst[i]->type == INST_FUNCTION)
            begin = get_node(st->inst[i]->name, FUNCTION, curr_class->functions);

        else if (st->inst[i]->type == INST_END_FUNCTION)
            begin = curr_class;

        else if (st->inst[i]->type == INST_END_CLASS)
            begin = NULL;

        tInstruction *instr = st->inst[i]->instr;

        if (st->inst[i]->type != INST_INSTRUCTION)
            continue;

        if (st->inst[i]->instr->op == INSTR_INSERT)
        {
            if (is_id(instr->addr1))
            {
                instr->addr1 = search_var(instr->addr1, begin, VARIABLE);
                if (instr->addr1 == NULL)
                {
                    throw_err(SEM_ERROR, UNDEF_VAR);
                    return false;
                }
            }
            else
            {
                instr->addr1 = make_data(instr->addr1);
            }
        }

        if (st->inst[i]->instr->op == ASSIGNMENT)
        {
            instr->addr3 = search_var(instr->addr3, begin, VARIABLE);
            if (instr->addr3 == NULL)
            {
                throw_err(SEM_ERROR, UNDEF_VAR);
                return false;
            }
        }

        if (st->inst[i]->instr->op == INSTR_ASS_ARG)
        {
            // najdeme uzel s funkci
            tNode *func = search_var(instr->addr3, begin, FUNCTION);
            if (func == NULL)
            {
                throw_err(SEM_ERROR, UNDEF_FUNC);
                return false;
            }

            if (current_arg == NULL)
                current_arg = get_argument(func->variables, 1);
            else
                current_arg = get_argument(func->variables, current_arg->argument + 1);

            // zavolali vetsi pocet argumentu, nez ktery funkce zpracovava
            if (current_arg == NULL)
            {
                throw_err(SEM_ERROR, CALL_FUNC_ARG);
                return false;
            }
            instr->addr3 = &current_arg->data;
        }

        if (st->inst[i]->instr->op == INSTR_CALL_FUNC)
        {
            bool full_id = false;
            if (strchr(strGetStr(instr->addr3), '.') != NULL)
                full_id = true;
            // najdeme uzel s funkci
            tNode *func = search_var(instr->addr3, curr_class, FUNCTION);
            if (func == NULL)
            {
                throw_err(SEM_ERROR, UNDEF_FUNC);
                return false;
            }

            // skontrolujeme jestli jsou predane funkci vsechny mozne argumenty
            if (current_arg && func->data.value.integer > current_arg->argument)
            {
                throw_err(SEM_ERROR, CALL_FUNC_ARG);
                return false;
            }
            // jinak nastavime ukazatel na nulu pro budouci zpracovani pomoci neho
            else
                current_arg = NULL;

            if (full_id)
            {
                // najdeme uzel s tridou
                instr->addr2 = search_var(instr->addr3, NULL, CLASS);
                if (instr->addr2 == NULL)
                {
                    throw_err(SEM_ERROR, UNDEF_CLASS);
                    return false;
                }
            }
            else
                instr->addr2 = curr_class;
            instr->addr3 = func;
        }
    }
    return true;
}

void *search_var(string *str, tNode *begin, int type)
{
    tNode *found;

    if (strchr(strGetStr(str), '.') != NULL)
    {
        char *ptr;
        string *var_name;

        ptr = strchr(strGetStr(str), '.');

        if ((var_name = malloc(sizeof(string))) == NULL)
        {
            throw_err(ALLOC_ERROR, ALL_STRUCT);
            return NULL;
        }
        strInit(var_name);

        for (char *i = ptr + 1; *i != '\0'; i++)
        {
            strAddChar(var_name, *i);
            *(i - 1) = 0;
        }
        tNode *class = get_node(str, CLASS, NULL);
        if (class == NULL)
        {
            throw_err(SEM_ERROR, UNDEF_CLASS);
            return NULL;
        }
        else
        {
            if (type == VARIABLE)
                found = get_node(var_name, VARIABLE, class->variables);
            else if (type == FUNCTION)
                found = get_node(var_name, FUNCTION, class->functions);
        }
    }
    else
    {
        if (type == VARIABLE)
            found = get_node(str, VARIABLE, begin->variables);
        else if (type == FUNCTION)
            found = get_node(str, FUNCTION, begin->functions);
        else if (type == CLASS)
            found = get_node(str, CLASS, NULL);
        else
        {
            throw_err(SEM_ERROR, ILLEGAL_OP);
            return NULL;
        }
    }

    if (found == NULL)
        return NULL;
    else if (type == VARIABLE)
        return &found->data;
    else if (type == FUNCTION || type == CLASS)
        return found;
    else
        return NULL;
}

tData *make_data(string *str)
{
    tData *new;
    if ((new = malloc(sizeof(tData))) == NULL)
    {
        throw_err(ALLOC_ERROR, ALL_STRUCT);
        return NULL;
    }
    new->type = get_type(str);

    switch (new->type)
    {
        case INT:;
            new->value.integer = atoi(strGetStr(str));
        break;
    }
    return new;
}

bool is_id(string *str)
{
    if (get_type(str) != -1 && strchr(strGetStr(str), '.') == NULL)
        return false;
    else
        return true;
}

int get_type(string *str)
{
    if (is_integer(str))
        return INT;
    else if (is_string(str))
         return STRING;
    else if (is_double(str))
        return DOUBLE;
    else if (is_boolean(str))
        return BOOLEAN;
    return -1;
}

bool is_integer(string *string)
{
    for (char *str = strGetStr(string); *str; str++)
    {
        if (!isdigit(*str))
            return false;
    }
    return true;
}

bool is_double(string *string)
{
    if (strchr(strGetStr(string), '.') != NULL)
        return true;
    else
        return false;
}

bool is_string(string *string)
{
    char *str = strGetStr(string);
    if (*str == '"' && str[string->length - 1] == '"')
        return true;
    else
        return false;
}

bool is_boolean(string *string)
{
    if (strcmp(strGetStr(string), "true") == 0 ||
        strcmp(strGetStr(string), "false") == 0)
        return true;
    else
        return false;
}

bool reverse_instr_stack(tInstrStack *st)
{
    tInstance *help;
    int top = st->top;
    int bottom = 0;

    while (top > bottom)
    {
        help = st->inst[bottom];
        st->inst[bottom++] = st->inst[top];
        st->inst[top--] = help;
    }
    return true;
}

/*	Inicializace zasobniku	*/
bool instr_stack_init(tInstrStack *st)
{
    main_instr_stack = st;
	main_instr_stack->top = -1;
	main_instr_stack->size = 10;
	if ((main_instr_stack->inst = malloc(main_instr_stack->size * sizeof(tInstruction *))) == NULL)
    {
		throw_err(ALLOC_ERROR, ALL_STRUCT);
		return false;
	}
	return true;
}

/*	Funkce uklada data na vrchol zasobniku	*/
bool instr_stack_push(tInstrStack *st, tInstance *data)
{
	if (!instr_stack_full(st))
    {
		st->top++;
		st->inst[st->top] = data;
	}
	else
    {
		st->size += 1;
		if ((st->inst = realloc(st->inst, st->size * sizeof(tInstance *))) == NULL)
        {
			throw_err(ALLOC_ERROR, ALL_STRUCT);
			return false;
		}
		if (!instr_stack_push(st, data))
			return false;
	}
	return true;
}

/*	Funkce odstrani polozku z vrcholu zasobniku a vrati ukazatel na nej 	*/
tInstance *instr_stack_pop(tInstrStack *st)
{
	tInstance *ret;
	if (!instr_stack_empty(st))
    {
		ret = st->inst[st->top];
		st->inst[st->top--] = NULL;
		return ret;
	}
	return  NULL;
}

/*	Funkce kontroluje je-li zasobnik plny	*/
bool instr_stack_full(tInstrStack *st)
{
    return st->top == st->size-1;
}

/*	Funkce kontroluje je-li zasobnik prazdny	*/
bool instr_stack_empty(tInstrStack *st)
{
    return st->top == -1;
}

/*  Funkce uvolni pamet zasobniku*/
void instr_free_stack(tInstrStack *st)
{
    if (!instr_stack_empty(st))
    {
		for (int i = st->top; i > -1; --i)
        {
            if (st->inst[i]->instr != NULL)
            {
                free(st->inst[i]->instr);
            }
            else
                if(st->inst[i]->name != NULL)
                    strFree(st->inst[i]->name);
			free(st->inst[i]);
		}
	}
	free(st->inst);
	free(st);
}

void print_instr_stack(tInstrStack *st)
{
    printf("\n\n");
	for (int i = st->top; i > -1; --i)
    {
        printf("Instruction stack item [%d] -> ", i);
        if (st->inst[i]->type == INST_CLASS)
        {
            printf("class with name \"%s\"\n", strGetStr(st->inst[i]->name));
        }
        else if (st->inst[i]->type == INST_FUNCTION)
        {
            printf("function with name \"%s\"\n", strGetStr(st->inst[i]->name));
        }
        else if (st->inst[i]->type == INST_INSTRUCTION)
        {
            printf("instruction with operation %d\n", st->inst[i]->instr->op);
        }
        else if (st->inst[i]->type == INST_END_CLASS)
        {
            printf("end class\n");
        }
        else if (st->inst[i]->type == INST_END_FUNCTION)
        {
            printf("end function\n");
        }
        else
        {
            printf("%s\n", "Unknown error!");
        }
    }
    printf("\n\n");
}
