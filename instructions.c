/* **************************************************************************/
/* Projekt:             Implementace interpretu jazyka IFJ16				*/
/* Predmet:             Formalni jazyky a prekladace (IFJ)					*/
/* Soubor:              instructions.c  (Prace s instrukcemi)				*/
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
#include "error.h"
#include "scanner.h"
#include "parser.h"
#include "instructions.h"
#include "table.h"
#include "frame.h"
#include "buffer.h"

tInstrStack *main_instr_stack;
tNode *begin;
tNode *curr_class;

// vytvari instance v instrukcnim listu
void create_instance(string *name, int type)
{
    tInstance *new_instance;
    if ((new_instance = malloc(sizeof(tInstance))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(new_instance);

    if (name != NULL)
    {
        if ((new_instance->name = malloc(sizeof(string))) == NULL ||
        strInit(new_instance->name))
            throw_err(INT_ERROR, ALL_STRUCT, 0);
        mark_mem(new_instance->name);
        strCopyString(new_instance->name, name);
    }
    else
        new_instance->name = NULL;
    new_instance->instr = NULL;

    // zachova informace
    new_instance->type = type;
    // dava na konec listu
    instr_stack_push(main_instr_stack, new_instance);
}

// vytvari instrukci
void create_instruction(int op, void *addr1, void *addr2, void *addr3)
{
    tInstruction *new_instr;

    if ((new_instr = malloc(sizeof(tInstruction))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(new_instr);
    new_instr->op = op;

    // записывает строки в адресса операндов
    // создаст новую строку для названия каждого операнда
    if (addr1 != NULL)
    {
        string *addr1_name = NULL;
        if ((addr1_name = malloc(sizeof(string))) == NULL)
            throw_err(INT_ERROR, ALL_STRUCT, 0);
        mark_mem(addr1_name);
        strInit(addr1_name);
        strCopyString(addr1_name, addr1);
        new_instr->addr1 = addr1_name;
    }
    else
        new_instr->addr1 = NULL;
    if (addr2 != NULL)
    {
        string *addr2_name = NULL;
        if ((addr2_name = malloc(sizeof(string))) == NULL)
            throw_err(INT_ERROR, ALL_STRUCT, 0);
        mark_mem(addr2_name);
        strInit(addr2_name);
        strCopyString(addr2_name, addr2);
        new_instr->addr2 = addr2_name;
    }
    else
        new_instr->addr2 = NULL;
    if (addr3 != NULL)
    {
        string *addr3_name = NULL;
        if ((addr3_name = malloc(sizeof(string))) == NULL)
            throw_err(INT_ERROR, ALL_STRUCT, 0);
        mark_mem(addr3_name);
        strInit(addr3_name);
        strCopyString(addr3_name, addr3);
        new_instr->addr3 = addr3_name;
    }
    else
        new_instr->addr3 = NULL;

    tInstance *new_inst;
    if ((new_inst = malloc(sizeof(tInstance))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(new_inst);
    new_inst->name = NULL;
    new_inst->type = INST_INSTRUCTION;
    new_inst->instr = new_instr;

    instr_stack_push(main_instr_stack, new_inst);
}

tData *make_data(string *str);
void *search_func(string *str, tNode *begin);

// создаст связи в таблице символов для определенных инструкций
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
        {
            curr_class = NULL;
            begin = NULL;
        }

        tInstruction *instr = st->inst[i]->instr;

        if (st->inst[i]->type != INST_INSTRUCTION)
            continue;

        if (instr->op == INSTR_INSERT)
        {            // addr2 - hodnota
            if (instr->addr2 != NULL)
                instr->addr2 = make_data(instr->addr2);
        }
        // pro instrukce potrebujeme index zacatku funkce v instrukcnim zasobniku
        else if (instr->op == INSTR_CALL_FUNC ||
                instr->op == INSTR_CALL_EXP_FUNC)
        {
            // zapiseme index do addr3
            proc_call_func(st, instr, instr->op);
        }
        // pro instrukce hledame uzel funkci
        // addr1 - nazev funkci
        else if (instr->op == CREATE_FRAME)
        {
            // zapiseme ukazatel na funkci do addr2, v addr1 bude nazev tridy
            instr->addr2 = search_func(instr->addr1, curr_class);
            // pokud volame funkce ve stejne tride, zapiseme nazev aktualni tridy
            if (equal_str(((string*)instr->addr1)->str, ((tNode *)instr->addr2)->key.str))
                instr->addr1 = &curr_class->key;
        }
        else if (instr->op >= INSTR_IF && instr->op <= INSTR_WHILE)
        {
            proc_ride_struct(st, instr, i);
        }

        else if (instr->op == INSTR_RETURN)
        {
            // vyhleda konec funkci
            int *indx;
            if ((indx = malloc(sizeof(int))) == NULL)
                throw_err(INT_ERROR, ALL_STRUCT, 0);
            mark_mem(indx);
            *indx = i;
            while (*indx > -1)
            {
                if (st->inst[*indx]->type == INST_END_FUNCTION)
                    break;
                *indx = *indx - 1;
            }
            instr->addr1 = indx;
        }
    }
}

void proc_call_func(tInstrStack *st, tInstruction *instr, int type)
{
    char *ptr = NULL;
    int *indx;
    if ((indx = malloc(sizeof(int))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(indx);
    *indx = -1;

    string *class_name;
    string *func_name;
    // vyhleda index zacatku tridy a potom funkci
    if ((ptr = strchr(strGetStr(instr->addr3), '.')) != NULL)
    {
        if ((func_name = malloc(sizeof(string))) == NULL)
            throw_err(INT_ERROR, ALL_STRUCT, 0);
        mark_mem(func_name);
        strInit(func_name);
        // rozdeli nazev funkci od nazvu tridy
        for (char *i = ptr + 1; *i; i++)
        {
            strAddChar(func_name, *i);
            *(i - 1) = 0;
        }
        class_name = instr->addr3;
        //printf("class name %s\n", class_name->str);
        //printf("func name %s\n", func_name->str);
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
    {
        //printf("------->>> proc_call_func, 1\n");
        throw_err(INT_ERROR, 0, 0);
    }

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
    {
        //printf("------->>> proc_call_func, 2\n");
        throw_err(INT_ERROR, 0, 0);
    }
    // kontrola navratove hodnoty pro funkce volane ve vyrazech
    // vyhledavame uzly tridy a funkci
    tNode *func = get_node(class_name, CLASS, NULL);
    if (func == NULL || (func = get_node(func_name, FUNCTION, func->functions)) == NULL
        || func == NULL)
        throw_err(SEM_ERROR, UNDEF_CLASS, func_name->str);

    if (type == INSTR_CALL_EXP_FUNC && func->type == VOID)
    {
        //printf("------->>> proc_call_func, 3\n");
        throw_err(UNINIT_ERROR, RET_VALUE, 0);
    }

    // index polozky funkci
    instr->addr3 = indx;
}

void proc_ride_struct(tInstrStack *st, tInstruction *instr, int beg_indx)
{
    int *indx;

    if (instr->op != INSTR_ELSE && instr->op != INSTR_END_BLCK)
    {
        if ((indx = malloc(sizeof(int))) == NULL)
            throw_err(INT_ERROR, ALL_STRUCT, 0);
        mark_mem(indx);
        *indx = -1;
    }

    int opened_blocks = 1;
    int i = beg_indx - 1;

    if (instr->op == INSTR_IF)
    {
        // ziskame konec bloku if
        while (i > 0 && opened_blocks)
        {
            if (st->inst[i]->type == INST_INSTRUCTION &&
                (st->inst[i]->instr->op == INSTR_IF ||
                st->inst[i]->instr->op == INSTR_ELSE ||
                st->inst[i]->instr->op == INSTR_WHILE))
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
                throw_err(INT_ERROR, ALL_STRUCT, 0);
            mark_mem(indx2);
            // index zacatku bloku else
            *indx2 = *indx - 1;
            instr->addr2 = indx2;
            // ziskame index konci bloku else pro instrukce else
            int *indx3;
            if ((indx3 = malloc(sizeof(int))) == NULL)
                throw_err(INT_ERROR, ALL_STRUCT, 0);
            mark_mem(indx3);
            i = *indx2 - 1;
            opened_blocks++;
            while (i > 0 && opened_blocks)
            {
                if (st->inst[i]->type == INST_INSTRUCTION &&
                    (st->inst[i]->instr->op == INSTR_IF ||
                    st->inst[i]->instr->op == INSTR_ELSE ||
                    st->inst[i]->instr->op == INSTR_WHILE))
                    opened_blocks++;

                if (st->inst[i]->type == INST_INSTRUCTION &&
                    st->inst[i]->instr->op == INSTR_END_BLCK)
                    opened_blocks--;

                if (!opened_blocks)
                    *indx3 = i;
                i--;
            }
            // index konci bloku else pro instrukce else
            st->inst[*indx-1]->instr->addr1 = indx3;
            //instr->addr3 = indx3;
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
            throw_err(INT_ERROR, ALL_STRUCT, 0);
        mark_mem(indx2);
        i = beg_indx - 1;
        while (i > 0 && opened_blocks)
        {
            if (st->inst[i]->type == INST_INSTRUCTION &&
                (st->inst[i]->instr->op == INSTR_IF ||
                st->inst[i]->instr->op == INSTR_ELSE ||
                st->inst[i]->instr->op == INSTR_WHILE))
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

// funkce vyhledava uzel funkci v binarnim strome
void *search_func(string *str, tNode *begin)
{
    // pomocny ukazatel na funkci
    tNode *found;
    // funkce muze byt z jine tridy, zkontrolujeme
    if (strchr(strGetStr(str), '.') != NULL)
    {
        string *func_name = split_name(str);
        // vyhledavame tridu
        tNode *class = get_node(str, CLASS, NULL);
        if (class == NULL)
        {
            //printf("------->>> search_var, 1\n");
            throw_err(SEM_ERROR, UNDEF_CLASS, str->str);
        }
        else
        {
            // vyhledavame funkci
            found = get_node(func_name, FUNCTION, class->functions);
            if (found == NULL)
            {
                //printf("------->>> search_var, 3\n");
                throw_err(SEM_ERROR, UNDEF_FUNC, func_name->str);
            }
        }
        strFree(func_name);
        free_pointer(func_name, true);
    }
    // funkce je z actualni tridy
    else
    {
        // vyhledavame funkci
        found = get_node(str, FUNCTION, begin->functions);
        if (found == NULL)
        {
            //printf("------->>> search_var, 5\n");
            throw_err(SEM_ERROR, UNDEF_FUNC, str->str);
        }
    }
    return found;
}

// funkce rozdeli plne kvalifikovany identifikator
string *split_name(string *id)
{
    // pomocny ukazatel na zacatek nazvu staticke promenne/funkce
    char *ptr = strchr(strGetStr(id), '.');
    // pomocny promenna nazvu staticke promenne/funkce
    string *help;
    if ((help = malloc(sizeof(string))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(help);
    strInit(help);

    // zapiseme nazev staticke promenne/funkce do promenne
    // v argumentu id zustane jenom nazev tridy
    for (char *i = ptr + 1; *i != '\0'; i++)
    {
        strAddChar(help, *i);
        *(i - 1) = 0;
    }
    return help;
}

tData *make_data(string *str)
{
    tData *new;
    if ((new = malloc(sizeof(tData))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(new);
    new->type = get_type(str);
    new->can_free = false;

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
            if (!strcmp(strGetStr(str), "true"))
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
    strFree(str);
    free_pointer(str, true);
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
    if ((strchr(strGetStr(string), 'e') != NULL || strchr(strGetStr(string), '.') != NULL ||
        strchr(strGetStr(string), 'E') != NULL) &&
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
    if (!strCmpConstStr(string, "true") ||
        !strCmpConstStr(string, "false"))
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
		throw_err(INT_ERROR, ALL_STRUCT, 0);
    mark_mem(main_instr_stack->inst);
}

/*	Funkce uklada data na vrchol zasobniku	*/
void instr_stack_push(tInstrStack *st, tInstance *data)
{
	if (!(st->top == st->size-1))
    {
		st->top++;
		st->inst[st->top] = data;
	}
	else
    {
		st->size += 1;
        free_pointer(st->inst, false);
		if ((st->inst = realloc(st->inst, st->size * sizeof(tInstance *))) == NULL)
			throw_err(INT_ERROR, ALL_STRUCT, 0);
        mark_mem(st->inst);
        instr_stack_push(st, data);
	}
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
    "INSTR_BEG_COND", "INSTR_IF", "INSTR_ELSE",
    "INSTR_CONTINUE", "INSTR_BREAK", "INSTR_END_BLCK",
    "INSTR_WHILE", "INSTR_RETURN", "INSTR_PRINT",
    "INSTR_R_INT", "INSTR_R_DOUBLE", "INSTR_R_STRING",
    "INSTR_LENGTH", "INSTR_SUBSTR", "INSTR_COMPARE",
    "INSTR_SORT", "INSTR_FIND", "INSTR_CALL_EXP_FUNC", "CREATE_FRAME",
    "DEFINE_VAR"
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
