#include <stdlib.h>
#include <stdbool.h>

#include "str.h"
#include "test_error.h"
#include "test_scanner.h"
#include "test_table_remake.h"
#include "test_inst.h"
#include "frame.h"

tFrameStack *st;

// inicializace noveho zasobniku ramce
tFrameStack *create_frame_stack()
{
    // alokujeme misto pro novy zasobnik
    //tFrameStack *st;
    if ((st = malloc(sizeof(tFrameStack))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    // alokujeme misto pro ukazatele na seznamy
    if ((st->stack = malloc(sizeof(tListData *))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);

    st->top = -1;
    st->alloc_size = 0;

    return st;
}

// vytvoreni noveho ramce
void create_frame(tFrameStack *st, tNode *node, char *name_class, int *arg_count)
{
    //printf("\n---------create----------");
    // alokujeme misto pro novy ramec
    tListData *new_list;
    if ((new_list = malloc(sizeof(tListData))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    new_list->name_frame = &node->key;
    new_list->name_par = name_class;
    // funkce obsahuje pocet argumentu
    new_list->numb_arg = node->argument;
    // zapiseme pocet ergumentu
    if (arg_count != NULL)
        *arg_count = new_list->numb_arg;
    new_list->type = node->node_type;
    // nastavime typ navratove hodnoty
    new_list->ret_type = node->type;

    new_list->Act = NULL;
    new_list->First = NULL;
    // vlozime novy seznam na vrchol zasobniku
    frame_stack_push(st, new_list);
    // naplnime seznam polozkami promennych
    fill_list(new_list, node->variables, node->node_type == CLASS);

    //print_frames(st);
    //printf("-------------------------\n\n");
}

// funkce nastavi index polozky v instrukcnim zasobniku kam se ma interpret
// vratit po ukonceni funkci
void set_ret_indx(int indx)
{
    st->stack[st->top]->last_pozition = indx;
}

// funkce vrati navratovy typ hodnoty ramci funkci na vrcholu zasobniku
int get_ret_val()
{
    return st->stack[st->top]->ret_type;
}

// zruseni ramce na vrcholu zasobniku
void destroy_frame(tFrameStack *st)
{
    //printf("\n--------destroy----------");
    // pomocne ukazatele
    tListData *help_list = st->stack[st->top];
    tVar *help_var;
    // zrusime kazdou polozku
    while (help_list->First != NULL)
    {
        // zachovani adresy pristi polozky seznamu
        help_var = help_list->First->next;
        // uvolni starou prvni polozku
        free(help_list->First);
        // nastavi novou
        help_list->First = help_var;
    }
    // zrusime seznam
    free(help_list);
    st->top--;

    //print_frames(st);
    //printf("-------------------------\n\n");
}

// vlozeni seznamu na vrchol zasobniku
void frame_stack_push(tFrameStack *st, tListData *list)
{
    if (!(st->top == st->alloc_size - 1))
    {
        // vlozime novy seznam na vrchol zasobniku
        st->top++;
        st->stack[st->top] = list;
    }
    // alokujeme misto v zasobniku pro novy ramec
    // pokud jeho nemame
    else
    {
        st->alloc_size++;
        if ((st->stack = realloc(st->stack, st->alloc_size * sizeof(tListData *))) == NULL)
            throw_err(INT_ERROR, ALL_STRUCT, 0);
        frame_stack_push(st, list);
    }
}

// naplneni seznamu polozkami promennych z binarniho stromu
void fill_list(tListData *list, tNode *begin, bool is_class)
{
    // funkce/trida muze nemit promenne
    if (begin == NULL)
        return;
    // alokujeme misto pro novou polozku
    tVar *new_var;
    if((new_var = malloc(sizeof(tVar))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);

    new_var->name_var = &begin->key;
    new_var->data.type = begin->type;
    if (new_var->data.type == STRING)
        strInit(&new_var->data.value.str);
    new_var->arg_order = begin->argument;
    new_var->data.can_free = false;
    new_var->is_init = false;

    // pokud uzel je trida -> definujeme statickou promennou
    if (is_class)
        new_var->is_def = true;
    else
        new_var->is_def = false;

    new_var->next = NULL;
    // vlozime polozku do seznamu
    insert_to_list(list, new_var);
    // jdeme dal po stromu
    if (begin->l_node != NULL)
        fill_list(list, begin->l_node, is_class);
    if (begin->r_node != NULL)
        fill_list(list, begin->r_node, is_class);
}

// vlozeni polozky do seznamu
void insert_to_list(tListData *list, tVar *var)
{
    // pokud seznam je prazdny -> vlozi na zacatek
    if (list->First == NULL)
        list->First = var;

    // jinak -> vlozi za aktivnim prvkem
    else if(list->Act != NULL)
    {
        // nastavime ukazatel polozky Act na novou polozku
        list->Act->next = var;
	}
    // nastavime aktivitu na novou polozku
    list->Act = var;
}

// hledani potrebne promenne
tData *get_value(tFrameStack *st, string *name, bool init)
{
    // pomocny ukazatel na nutny seznam
    tListData *help = NULL;
    // pokud volame promennou tridy -> vyhledavame tridu
    if (strchr(strGetStr(name), '.') != NULL)
    {
        // pomocna promenna nazvu staticke promenne
        string *var_name = split_name(name);
        // vyhledavame seznam promennych podle nazvu tridy
        for (int i = st->top; i > -1; i--)
        {
            if (equal_str(st->stack[i]->name_frame->str, name->str) &&
                st->stack[i]->type == CLASS)
                help = st->stack[i];
        }
        if (help == NULL)
            throw_err(SEM_ERROR, UNDEF_CLASS, name->str);
        // nastavime name na var_name
        name = var_name;
    }
    // jinak vyhledavame v seznamu na vrcholu zasobniku
    else
        help = st->stack[st->top];
    help->Act = help->First;

    // vyhledame promennou v seznamu
    while (help->Act != NULL)
    {
        if (equal_str(help->Act->name_var->str, name->str))
            break;
        help->Act = help->Act->next;
    }
    // nenasli jsme -> zkusime vyhlednout ve tride
    if (help->Act == NULL)
    {
        // hledame tridu podle nazvu name_par ve funkci
        for (int i = st->top; i > -1; i--)
        {
            if (equal_str(st->stack[i]->name_frame->str, help->name_par) &&
                st->stack[i]->type == CLASS)
                help = st->stack[i];
        }
        help->Act = help->First;
        // vyhledame promennou v seznamu
        while (help->Act != NULL)
        {
            if (equal_str(help->Act->name_var->str, name->str))
                break;
            help->Act = help->Act->next;
        }
        // nenasli jsme aj ve tride
        if (help->Act == NULL)
        {
            //printf("----------------->frame, %s\n", "ok");
            //print_frames(st);
            throw_err(SEM_ERROR, UNDEF_VAR, name->str);
        }
    }

    // inicializace promenne pokud potrebujeme
    if (init)
        help->Act->is_init = true;
    // pokud volame promennou ktera nebyla definovana
    // nebo nebyla inicializovana -> zavolame chybu
    else if (!help->Act->is_def)
    {
        //printf("------------->frame, 2\n");
        throw_err(SEM_ERROR, UNDEF_VAR, help->Act->name_var->str);
    }
    else if (!help->Act->is_init)
        throw_err(UNINIT_ERROR, UNINIT_VAR, help->Act->name_var->str);

    return &help->Act->data;
}

// funkce vrati polozku tData urciteho argumentu
tData *get_arg(int number)
{
    // bylo zadano vice argumentu nez ma funkce
    if (number < 1)
        throw_err(SEM_TYPE_ERROR, CALL_FUNC_ARG, st->stack[st->top]->name_frame->str);

    // vyhledavame argument v ramci na vrcholu zasobniku
    tListData *help = st->stack[st->top];
    help->Act = help->First;

    // vyhledame argument v seznamu
    while (help->Act != NULL)
    {
        if (help->Act->arg_order == number)
            break;
        help->Act = help->Act->next;
    }

    help->Act->is_def = true;
    help->Act->is_init = true;
    return &help->Act->data;
}

// funkce nastavi flag definici promenne
void def_var(string *name)
{
    // vyhledavame promennou v ramci na vrcholu zasobniku
    tListData *help = st->stack[st->top];
    help->Act = help->First;

    // vyhledame promennou v seznamu
    while (help->Act != NULL)
    {
        if (equal_str(help->Act->name_var->str, name->str))
            break;
        help->Act = help->Act->next;
    }
    // nenasli jsme
    if (help->Act == NULL)
        throw_err(SEM_ERROR, UNDEF_VAR, name->str);
    // nastavime flag
    help->Act->is_def = true;
}

// vypis ramci
void print_frames(tFrameStack *st)
{
    int top = st->top;
    while (top > -1)
    {
        printf("\nFrame [%d]:\n", top);
        tListData *list = st->stack[top];
        list->Act = list->First;
        printf("    List, function name [%s]:\n", list->name_frame->str);
        while (list->Act != NULL)
        {
            printf("        name variable [%s], ", list->Act->name_var->str);

            if (list->Act->is_init)
            {
                switch (list->Act->data.type)
                {
                    case INT:;
                    printf("typ INT, value [%d]\n", list->Act->data.value.integer);
                    break;
                    case DOUBLE:;
                    printf("typ DOUBLE, value [%g]\n", list->Act->data.value.real);
                    break;
                    case STRING:;
                    printf("typ STRING, value [%s]\n", list->Act->data.value.str.str);
                    break;
                    case BOOLEAN:;
                    printf("typ BOOLEAN, value [%d]\n", list->Act->data.value.boolean);
                    break;
                }
            }
            else
                printf("uninitialized\n");

            list->Act = list->Act->next;
        }
        top--;
    }
}
