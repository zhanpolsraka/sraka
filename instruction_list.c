/** prototype
*** instruction_list.c
*** IFJ 2016
**/
#include "instruction_list.h"

/**
*** Create a new instruction class and add it into global class list.
*** WARNING: Exception handling is missing! The function returns:
*** true if everything is OK
*** false if something isn't OK (malloc error)
**/
bool gen_class(string *name) {
    /* As always we need something to work with. */
    if (g_class_list == NULL) {
        if(!create_and_init_g_class_list()) {
            return false;
        }
    }
    /* We need to allocate memory for our new class. */
    tClass *new_class = malloc(sizeof(tClass));
    if (new_class == NULL) {
        return false;
    }
    /* Also we need to allocate memory for its name. */
    new_class->name = malloc(sizeof(string));
    /* DTTO for its instruction list. */
    new_class->instr_list = create_and_init_instr_list();
    if (new_class->instr_list == NULL) {
        return false;
    }
    /* DTTO for its function list. */
    new_class->func_list = create_and_init_func_list();
    if (new_class->func_list == NULL) {
        return false;
    }
    /* Inicialize it. */
    strCopyString(new_class->name, name);
    new_class->vars = get_node(name, VARIABLE, root)->variables;
    new_class->prev_class = NULL;
    new_class->next_class = NULL;
    /* Now we can add it into our global class list. */
    if (g_class_list->first == NULL) {
        g_class_list->first = new_class;
        g_class_list->current = g_class_list->first;
        g_class_list->last = g_class_list->first;
    }
    else {
        tClass *temp = g_class_list->current;
        g_class_list->current = g_class_list->last;
        g_class_list->current->next_class = new_class;
        g_class_list->current->next_class->prev_class = g_class_list->current->next_class;
        g_class_list->last = g_class_list->last->next_class;
        g_class_list->current = temp;
    }
    return true;
}

/**
*** Create a new function instance and add it into current function list of current class.
*** WARNING: Exception handling is missing! The function returns:
*** true if everything is OK
*** false if something isn't OK (malloc error or there is no active class)
**/
bool gen_function(string *name) {
    /* We need something to work with. */
    if (g_class_list->current != NULL) {
        /* Let's create a new function instance. */
        tFunction *new_function = malloc(sizeof(tFunction));
        if (new_function == NULL) {
            return false;
        }
        /* We also need to allocate memory for its name. */
        new_function->name = malloc(sizeof(string));
        if (new_function->name == NULL) {
            return false;
        }
        /* DTTO for its instruction list. */
        new_function->instr_list = create_and_init_instr_list();
        if (new_function->instr_list == NULL) {
            return false;
        }
        /* Now we inicialize it. */
        strCopyString(new_function->name, name);
        new_function->vars = get_node(name, VARIABLE, root)->variables;
        new_function->next_func = NULL;
        /* So now we're adding our function into function list of current class. */
        if (g_class_list->current->func_list->first == NULL) {
            g_class_list->current->func_list->first = new_function;
            g_class_list->current->func_list->current = g_class_list->current->func_list->first;
        }
        else {
            g_class_list->current->func_list->current->next_func = new_function;
            g_class_list->current->func_list->current = g_class_list->current->func_list->current->next_func;
        }
        return true;
    }
    return false;
}

/**
*** Create a new instruction and add it into current instruction list of current class\function.
*** WARNING: Exception handling is missing! The function returns:
*** true if everything is OK
*** false if something isn't OK (malloc error or there is no active class)
**/
bool gen_instruction(tType operation, void *first_addr, void *second_addr, void *third_addr) {
    /* As always we need something to work with. */
    if (g_class_list->current != NULL) {
        /* Ok. Now we allocate memory for a new instruction. */
        tInstruction *new_instr = malloc(sizeof(tInstruction));
        if (new_instr == NULL) {
            return false;
        }
        /* Initialize it. */
        new_instr->op = operation;
        new_instr->first_address = first_addr;
        new_instr->second_address = second_addr;
        new_instr->third_address = third_addr;
        new_instr->next = NULL;
        /* Now we need to understand where are we:
        ** In the class instruction list or
        ** In the instruction list of its active function */
        if (g_class_list->current->func_list->current == NULL) {
            /* We are in the class instruction list.
            ** Ok. So now we add our instruction into class instruction list. */
            if (g_class_list->current->instr_list->first == NULL) {
                g_class_list->current->instr_list->first = new_instr;
                g_class_list->current->instr_list->current = g_class_list->current->instr_list->first;
            }
            else {
                g_class_list->current->instr_list->current->next = new_instr;
                g_class_list->current->instr_list->current = g_class_list->current->instr_list->current->next;
            }
        }
        else {
            /* We are in the instruction list of the current class' active function.
            ** Ok. So now we add our instruction into active function's instruction list of the current class. */
            if (g_class_list->current->func_list->current->instr_list->first == NULL) {
                g_class_list->current->func_list->current->instr_list->first = new_instr;
                g_class_list->current->func_list->current->instr_list->current = g_class_list->current->func_list->current->instr_list->first;
            }
            else {
                g_class_list->current->func_list->current->instr_list->current->next = new_instr;
                g_class_list->current->func_list->current->instr_list->current = g_class_list->current->func_list->current->instr_list->current->next;
            }
        }
        return true;
    }
    return false;
}

/**
*** The list "methods" for classes.
**/
bool create_and_init_g_class_list() {

  g_class_list = malloc(sizeof(tClassList));
  if (g_class_list == NULL) {
    return false;
  }
  g_class_list->first = NULL;
  g_class_list->current = NULL;
  g_class_list->last = NULL;
  return true;
}

void destroy_g_class_list() {
  while (g_class_list->first != NULL) {
    tClass *class_to_delete = g_class_list->first;
    g_class_list->first = class_to_delete->next_class;
    destroy_instr_list(class_to_delete->instr_list);
    destroy_func_list(class_to_delete->func_list);
    strFree(class_to_delete->name);
    free(class_to_delete);
  }
  g_class_list->current = NULL;
  g_class_list->first = NULL;
  g_class_list->last = NULL;
  free(g_class_list);
}

/**
*** The list "methods" for functions.
**/

/**
*** Makes pointer to current function unavailable.
*** Call it when you're out of function.
**/
void function_out() {
    if (g_class_list->current != NULL) {
        if (g_class_list->current->func_list->current != NULL) {
            g_class_list->current->func_list->current = NULL;
        }
    }
}

tFuncList *create_and_init_func_list() {

    tFuncList *new_func_list = malloc(sizeof(tFuncList));
    if (new_func_list == NULL) {
        return NULL;
    }
    new_func_list->first = NULL;
    new_func_list->current = new_func_list->first;
    return new_func_list;
}

void destroy_func_list(tFuncList *list) {

    while(list->first != NULL) {
        tFunction *func_to_delete = list->first;
        list->first = func_to_delete->next_func;
        strFree(func_to_delete->name);
        destroy_instr_list(func_to_delete->instr_list);
        free(func_to_delete);
    }
    list->first = NULL;
    list->current = NULL;
    free(list);
}

/**
*** The list "methods" for instructions.
**/
tInstrList *create_and_init_instr_list() {

    tInstrList *new_instr_list = malloc(sizeof(tInstrList));
    if (new_instr_list == NULL) {
        return NULL;
    }
    new_instr_list->first = NULL;
    new_instr_list->current = new_instr_list->first;
    return new_instr_list;
}

void destroy_instr_list(tInstrList *list) {

  while (list->first != NULL) {
    tInstruction *instr_to_delete = list->first;
    list->first = instr_to_delete->next;
    free(instr_to_delete);
  }
  list->current = NULL;
  list->first = NULL;
  free(list);
}
