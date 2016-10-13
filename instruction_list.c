/** prototype
*** instruction_list.c
*** IFJ 2016
**/
#include "instruction_list.h"

/**
*** Create a new instruction class and add it into instr. class list.
*** WARNING: Exception handling is missing! The function returns:
*** 1 if everything is OK
*** 0 if something isn't OK (malloc error)
**/
int gen_instr_class(t_class_list *list, string *name, t_instr_list *instr_list) {

  t_instr_class *new_class = malloc(sizeof(t_instr_class));
  if (new_class == NULL) {
    return FAILURE;
  }

  new_class->name = name;
  new_class->prev_class = NULL;
  new_class->next_class = NULL;
  new_class->instr_list = instr_list;

  if (list->first == NULL) {
    list->first = new_class;
    list->current = list->first;
    list->last = list->first;
  }
  else {
    t_instr_class *temp = list->current;
    list->current = list->last;
    list->current->next_class = new_class;
    list->current->next_class->prev_class = list->current->next_class;
    list->last = list->last->next_class;
    list->current = temp;
  }
  return SUCCESS;
}
/**
*** Create a new instruction and add it into instruction list.
*** WARNING: Exception handling is missing! The function returns:
*** 1 if everything is OK
*** 0 if something isn't OK (malloc error)
**/
int gen_instruction(t_instr_list *list, t_operation op, void *first_addr, void *second_addr, void *third_addr) {

  t_instruction *new_instr = malloc(sizeof(t_instruction));
  if (new_instr == NULL) {
      return FAILURE;
  }
  new_instr->op = op;
  new_instr->first_address = first_addr;
  new_instr->second_address = second_addr;
  new_instr->third_address = third_addr;
  new_instr->next = NULL;

  if (list->first == NULL) {
    list->first = new_instr;
    list->current = list->first;
  }
  else {
    list->current->next = new_instr;
    list->current = list->current->next;
  }
  return SUCCESS;
}

/**
*** The list "methods" for classes.
**/
t_class_list *create_and_init_class_list() {

  t_class_list *new_class_list = malloc(sizeof(t_class_list));
  if (new_class_list == NULL) {
    return NULL;
  }
  new_class_list->first = NULL;
  new_class_list->current = NULL;
  new_class_list->last = NULL;
  return new_class_list;
}
void destroy_class_list(t_class_list *list) {

  while (list->first != NULL) {
    t_instr_class *elem_to_delete = list->first;
    list->first = elem_to_delete->next_class;
    destroy_instr_list(elem_to_delete->instr_list);
    free(elem_to_delete);
  }
  list->current = NULL;
  list->first = NULL;
  list->last = NULL;
}

/**
*** The list "methods" for instructions.
**/
t_instr_list *create_and_init_instr_list() {

  t_instr_list *new_instr_list = malloc(sizeof(t_instr_list));
  if (new_instr_list == NULL) {
    return NULL;
  }
  new_instr_list->first = NULL;
  new_instr_list->current = new_instr_list->first;
  return new_instr_list;
}

void destroy_instr_list(t_instr_list *list) {

  while (list->first != NULL) {
    t_instruction *elem_to_delete = list->first;
    list->first = elem_to_delete->next;
    free(elem_to_delete);
  }
  list->current = NULL;
  list->first = NULL;
}
