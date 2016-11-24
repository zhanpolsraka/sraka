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
tNode *begin;
tNode *curr_class;
tNode *current_arg;

// vytvari instance v instrukcnim listu
void create_instance(string *name, int type)
{
    tInstance *new_instance;
    if ((new_instance = malloc(sizeof(tInstance))) == NULL)
        throw_err(ALLOC_ERROR, ALL_STRUCT);

    if (name != NULL)
    {
        if ((new_instance->name = malloc(sizeof(string))) == NULL ||
        strInit(new_instance->name))
            throw_err(ALLOC_ERROR, ALL_STRUCT);

        strCopyString(new_instance->name, name);
    }
    // zachova informace
    new_instance->instr = NULL;
    new_instance->type = type;
    // dava na konec listu
    instr_stack_push(main_instr_stack, new_instance);
}

// vytvari instrukci
void create_instruction(int op, void *addr1, void *addr2, void *addr3)
{
    tInstruction *new_instr;

    if ((new_instr = malloc(sizeof(tInstruction))) == NULL)
        throw_err(ALLOC_ERROR, ALL_STRUCT);

    new_instr->op = op;

    // zapisuje retezce do adres operandu
    // (vytvari novy retezec pro nazev operandu)
    if (addr1 != NULL)
    {
        string *addr1_name = NULL;
        if ((addr1_name = malloc(sizeof(string))) == NULL)
            throw_err(ALLOC_ERROR, ALL_STRUCT);

        strInit(addr1_name);
        strCopyString(addr1_name, addr1);
        new_instr->addr1 = addr1_name;
    }
    if (addr2 != NULL)
    {
        string *addr2_name = NULL;
        if ((addr2_name = malloc(sizeof(string))) == NULL)
            throw_err(ALLOC_ERROR, ALL_STRUCT);

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

        throw_err(ALLOC_ERROR, ALL_STRUCT);

    new_inst->type = INST_INSTRUCTION;
    new_inst->instr = new_instr;

    instr_stack_push(main_instr_stack, new_inst);

}

void *search_var(string *str, tNode *begin, int type);
tData *make_data(string *str);

// vytvari relaci v tabulke symbolu prislusne instrukce
void make_relations(tInstrStack *st)
{
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

        if (instr->op == INSTR_INSERT)
        {
            if (is_id(instr->addr1))
            {
                instr->addr1 = search_var(instr->addr1, begin, VARIABLE);
                if (instr->addr1 == NULL)
                    throw_err(SEM_ERROR, UNDEF_VAR);
            }
            else
            {
                instr->addr1 = make_data(instr->addr1);
            }
        }

        else if (instr->op == ASSIGNMENT)
        {
            instr->addr3 = search_var(instr->addr3, begin, VARIABLE);
            if (instr->addr3 == NULL)
                throw_err(SEM_ERROR, UNDEF_VAR);
        }

        else if (instr->op == INSTR_ASS_ARG)
        {
            // najdeme uzel s funkci
            tNode *func = search_var(instr->addr3, curr_class, FUNCTION);
            if (func == NULL)
                throw_err(SEM_ERROR, UNDEF_FUNC);

            // najdeme argument ve funkci
            if (current_arg == NULL)
                current_arg = get_argument(func->variables, 1);
            else
                current_arg = get_argument(func->variables, current_arg->argument + 1);

            // zavolali vetsi pocet argumentu, nez ktery funkce zpracovava
            if (current_arg == NULL)
                throw_err(SEM_ERROR, CALL_FUNC_ARG);

            instr->addr3 = &current_arg->data;
        }

        else if (instr->op == INSTR_CALL_FUNC)
        {
            proc_call_func(st, instr);
        }

        else if (instr->op >= INSTR_IF && instr->op <= INSTR_WHILE)
        {
            proc_ride_struct(st, instr, i);
        }
    }
}

void proc_call_func(tInstrStack *st, tInstruction *instr)
{
    char *ptr = NULL;
    int *indx;
    if ((indx = malloc(sizeof(int))) == NULL)
        throw_err(ALLOC_ERROR, ALL_STRUCT);
    *indx = -1;

    string *class_name;
    string *func_name;
    // vyhleda index zacatku tridy a potom funkci
    if ((ptr = strchr(strGetStr(instr->addr3), '.')) != NULL)
    {
        if ((func_name = malloc(sizeof(string))) == NULL)
            throw_err(ALLOC_ERROR, ALL_STRUCT);
        strInit(func_name);
        // rozdeli nazev funkci od nazvu tridy
        for (char *i = ptr + 1; *i; i++)
        {
            strAddChar(func_name, *i);
            *(i - 1) = 0;
        }
        class_name = instr->addr3;
    }
    else
    {
        class_name = &curr_class->key;
        func_name = instr->addr3;
    }
    // vyhleda instrukce zacatku tridy
    int i = st->top;
    while (i > -1)
    {
        if (st->inst[i]->type == INST_CLASS &&
            equal_str(st->inst[i]->name->str, class_name->str))
        {
            *indx = i;
            break;
        }
        i--;
    }
    if (*indx == -1)
        throw_err(SEM_ERROR, UNDEF_CLASS);

    // vyhleda zacatek funkci
    int old_indx = *indx;
    while (i > -1)
    {
        if (st->inst[i]->type == INST_FUNCTION &&
            equal_str(st->inst[i]->name->str, func_name->str))
        {
            *indx = i;
            break;
        }
        i--;
    }
    if (*indx == old_indx)
        throw_err(SEM_ERROR,UNDEF_FUNC);
    *indx = *indx -1;
    instr->addr3 = indx;
}

void proc_ride_struct(tInstrStack *st, tInstruction *instr, int beg_indx)
{
    int *indx;
    if ((indx = malloc(sizeof(int))) == NULL)
        throw_err(ALLOC_ERROR, ALL_STRUCT);
    *indx = -1;

    int opened_blocks = 1;
    int i = beg_indx - 1;

    if (instr->op == INSTR_IF)
    {
        // ziskame konec bloku if
        while (i > 0 && opened_blocks)
        {
            if (st->inst[i]->type == INST_INSTRUCTION &&
                (st->inst[i]->instr->op == INSTR_IF ||
                st->inst[i]->instr->op == INSTR_ELSE))
                opened_blocks++;

            if (st->inst[i]->type == INST_INSTRUCTION &&
                st->inst[i]->instr->op == INSTR_END_BLCK)
                opened_blocks--;

            if (!opened_blocks)
                *indx = i;
            i--;
        }
        // index konce bloku if
        instr->addr1 = indx;
        // pokud pristi instrukce else
        // ziskame indexy zacatku a konce bloku
        if (st->inst[*indx-1]->type == INST_INSTRUCTION &&
            st->inst[*indx-1]->instr->op == INSTR_ELSE)
        {
            int *indx2;
            if ((indx2 = malloc(sizeof(int))) == NULL)
                throw_err(ALLOC_ERROR, ALL_STRUCT);
            // index zacatku bloku else
            *indx2 = *indx - 1;
            instr->addr2 = indx2;
            // ziskame index konci bloku else
            int *indx3;
            if ((indx3 = malloc(sizeof(int))) == NULL)
                throw_err(ALLOC_ERROR, ALL_STRUCT);

            i = *indx2 - 1;
            opened_blocks++;
            while (i > 0 && opened_blocks)
            {
                if (st->inst[i]->type == INST_INSTRUCTION &&
                    (st->inst[i]->instr->op == INSTR_IF ||
                    st->inst[i]->instr->op == INSTR_ELSE))
                    opened_blocks++;

                if (st->inst[i]->type == INST_INSTRUCTION &&
                    st->inst[i]->instr->op == INSTR_END_BLCK)
                    opened_blocks--;

                if (!opened_blocks)
                    *indx3 = i;
                i--;
            }
            // index konci bloku else
            instr->addr3 = indx3;
        }
        else
        {
            instr->addr2 = NULL;
            instr->addr3 = NULL;
        }
    }
    else if (instr->op == INSTR_WHILE)
    {
        i++;
        // ziskame index zacatku podminky
        while (st->inst[i]->instr->op != INSTR_BEG_COND)
            i++;
        *indx = i;
        // index zacatku podminky
        instr->addr1 = indx;
        // ziskame index konce bloku while
        int *indx2;
        if ((indx2 = malloc(sizeof(int))) == NULL)
            throw_err(ALLOC_ERROR, ALL_STRUCT);
        i = beg_indx - 1;
        while (i > 0 && opened_blocks)
        {
            if (st->inst[i]->type == INST_INSTRUCTION &&
                (st->inst[i]->instr->op == INSTR_IF ||
                st->inst[i]->instr->op == INSTR_ELSE))
                opened_blocks++;

            if (st->inst[i]->type == INST_INSTRUCTION &&
                st->inst[i]->instr->op == INSTR_END_BLCK)
                opened_blocks--;

            if (!opened_blocks)
                *indx2 = i;
            i--;
        }
        // index konci bloku while
        instr->addr2 = indx2;
    }
    else if (instr->op == INSTR_BREAK)
    {
        // ziskame index zacatku bloku while
        while (st->inst[i]->instr->op != INSTR_WHILE)
            i++;
        // vezmeme index konci bloku while
        *indx = *(int *)st->inst[i]->instr->addr2;
        instr->addr1 = indx;
    }
    else if (instr->op == INSTR_CONTINUE)
    {
        // ziskame index zacatku bloku while
        while (st->inst[i]->instr->op != INSTR_WHILE)
            i++;
        // vezmeme index podminky bloku while
        *indx = *(int *)st->inst[i]->instr->addr1;
        instr->addr1 = indx;
    }
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
            throw_err(ALLOC_ERROR, ALL_STRUCT);
        strInit(var_name);

        for (char *i = ptr + 1; *i != '\0'; i++)
        {
            strAddChar(var_name, *i);
            *(i - 1) = 0;
        }

        tNode *class = get_node(str, CLASS, NULL);
        if (class == NULL)
            throw_err(SEM_ERROR, UNDEF_CLASS);
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
        {
            //print_tree();
            found = get_node(str, FUNCTION, begin->functions);
        }
        else
            throw_err(SEM_ERROR, ILLEGAL_OP);
    }

    if (found == NULL)
        return NULL;
    else if (type == VARIABLE)
        return &found->data;
    else if (type == FUNCTION)
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

        case DOUBLE:;
            new->value.real = atof(strGetStr(str));
        break;

        case STRING:;
            // zbavi se apostrofy a prepise retezec
            for (int i = 1; i < str->length - 1; i++)
            {
                str->str[i - 1] = str->str[i];
            }

            str->str[str->length - 1] = 0;
            str->str[str->length - 2] = 0;
            str->length -= 2;

            strInit(&new->value.str);
            strCopyString(&new->value.str, str);
        break;

        case BOOLEAN:;
            if (!strcmp(strGetStr(str), "\"true\""))
                new->value.boolean = true;
            else
                new->value.boolean = false;
        break;

        case SYMB:;
            // zbavi se apostrofy a prepise symbol
            str->str[0] = str->str[1];

            str->str[str->length - 1] = 0;
            str->str[str->length - 2] = 0;
            str->length -= 2;

            strInit(&new->value.str);
            strCopyString(&new->value.str, str);
        break;
    }
    return new;
}

bool is_id(string *str)
{
    if (get_type(str) == -1 && (isalpha(str->str[0]) ||
        str->str[0] == '_' || str->str[0] == '$'))
            return true;
    else
        return false;
}

int get_type(string *str)
{
    if (is_integer(str))
        return INT;
    else if (is_boolean(str))
        return BOOLEAN;
    else if (is_string(str))
        return STRING;
    else if (is_double(str))
        return DOUBLE;
    else if (is_symb(str))
        return SYMB;
    else
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
    if (strchr(strGetStr(string), '.') != NULL &&
        strGetLength(string) != 1 && isdigit(string->str[0]))
        return true;
    else
        return false;
}

bool is_string(string *string)
{
    char *str = string->str;
    if (*str == '"' && str[string->length - 1] == '"')
        return true;
    else
        return false;
}

bool is_symb(string *string)
{
    char *str = strGetStr(string);
    if (*str == '\'' && str[string->length - 1] == '\'')
        return true;
    else
        return false;
}

bool is_boolean(string *string)
{
    if (!strCmpConstStr(string, "\"true\"") ||
        !strCmpConstStr(string, "\"false\""))
        return true;
    else
        return false;
}

void reverse_instr_stack(tInstrStack *st)
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
}

/*	Inicializace zasobniku	*/
void instr_stack_init(tInstrStack *st)
{
    main_instr_stack = st;
	main_instr_stack->top = -1;
	main_instr_stack->size = 10;
	if ((main_instr_stack->inst = malloc(main_instr_stack->size * sizeof(tInstruction *))) == NULL)
		throw_err(ALLOC_ERROR, ALL_STRUCT);
}

/*	Funkce uklada data na vrchol zasobniku	*/
void instr_stack_push(tInstrStack *st, tInstance *data)
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
			throw_err(ALLOC_ERROR, ALL_STRUCT);

        instr_stack_push(st, data);
	}
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

char *name_inst[] =
{
    0,
    "PLUS", "MINUS", "MUL", "DIV",
    0, 0, 0, 0, "COMPARISON", "LESS",
    "GREATER", "LOEQ", "GOEQ", "NEQ",
    "NOT", 0, 0, "OR", "AND", 0, 0, 0,
    0, 0, 0, 0, 0, "ASSIGNMENT", "INCREMENT",
    "DECREMENT", 0, 0, 0, 0, "INSTR_CALL_FUNC",
     "INSTR_ASS_ARG", "INSTR_INSERT",
    "INSTR_MOD", "INSTR_INC", "INSTR_DEC",
    "INSTR_BEG_COND", "INSTR_IF", "INSTR_ELSE",
    "INSTR_CONTINUE", "INSTR_BREAK",
    "INSTR_WHILE", "INSTR_END_BLCK", "INSTR_PRINT",
    "INSTR_R_INT", "INSTR_R_DOUBLE", "INSTR_R_STRING",
    "INSTR_LENGTH", "INSTR_SUBSTR", "INSTR_COMPARE",
    "INSTR_SORT"
};

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
            printf("instruction with operation %d -> %s\n", st->inst[i]->instr->op, name_inst[st->inst[i]->instr->op]);
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
