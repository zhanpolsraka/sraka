#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "str.h"
#include "test_inst.h"
#include "test_table.h"
#include "test_scanner.h"
#include "interpret.h"

tDStack ds;

int execute(tInstrStack *s){
	dStackInit(&ds);
	for(int i = s->top; i > -1; i--) {
		if(s->inst[i]->type == INST_INSTRUCTION && s->inst[i]->instr->op == INSTR_INSERT) {
			printf("Found INSTR_INSERT, pushing to DStack\n"); //rm
			//void *addr = s->inst[i]->instr->addr1;
			dStackPush(&ds, (tData *)(s->inst[i]->instr->addr1));
			dStackPrint(&ds);
		}
		else if(s->inst[i]->type == INST_INSTRUCTION && s->inst[i]->instr->op == ASSIGNMENT) {
			printf("--->ASSIGNMENT %d\n", s->inst[i]->instr->op); //rm
		}
		else if(s->inst[i]->type == INST_CLASS && !strcmp(strGetStr(s->inst[i]->name), "Main")) {
			printf("Found class Main (instance type - [%d], name - [%s])\n", s->inst[i]->type, strGetStr(s->inst[i]->name)); //rm
		}
		else if(s->inst[i]->type == INST_FUNCTION && !strcmp(strGetStr(s->inst[i]->name), "run")) {
			printf("Found proccess run (instance type - [%d], name - [%s])\n", s->inst[i]->type, strGetStr(s->inst[i]->name)); //rm
		}
		else if(s->inst[i]->type == INST_INSTRUCTION) {
			printf("Found instruction with operation %d\n", s->inst[i]->instr->op); //rm
		}
		else if(s->inst[i]->type == INST_END_CLASS || s->inst[i]->type == INST_END_FUNCTION) {
			printf("Found end of function/class\n"); //rm
		}
	}
	return 0;
}

void dStackInit(tDStack *s) {
	if (s != NULL) {
		s->top = -1;
	}
	else {
		//err
	}
}

void dStackPrint(tDStack *s) {
	for(int i = s->top; i > -1; i--) {
		printf("N%d: type is %d, values is %d\n", i, s->arr[i]->type, s->arr[i]->value);
	}
}

void dStackPush(tDStack *s, tData *data) {
	if (s != NULL && s->top < MAX_STACK) {
		s->top++;
		s->arr[s->top] = data;
	}
	else {
		//err
	}
}