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
	//Top of instructions of method run of class Main
	int run = -1;
	bool inMain = false;
	for(int i = s->top; i > -1; i--) {
		if(s->inst[i]->type == INST_CLASS && !strCmpConstStr(s->inst[i]->name, "Main")) {
			inMain = true;
			continue;
		}
		if(s->inst[i]->type == INST_FUNCTION && !strCmpConstStr(s->inst[i]->name, "run") && inMain) {
			run = i;
			break;
		}
	}
	//Check if error occured
	if (run == -1) {
		//err
		printf("Could not find run in Main, aborting execution\n");
		return -1;
	}
	for(int i = run; s->inst[i]->type != INST_END_CLASS; i--) {
		executeInst(s->inst[i]);
	}
	return 0;
}

//Execute single instruction
void executeInst(tInstance *inst) {
	if (inst->type == INST_INSTRUCTION) printf("Executing instruction with with opcode [%d]\n", inst->instr->op);
	//----------------------------------------------------------------------------------------
	else if (inst->type == INST_CLASS) printf("Instance class, name [%s]\n", strGetStr(inst->name));
	else if (inst->type == INST_FUNCTION) printf("Instance func, name [%s]\n", strGetStr(inst->name));
	else if (inst->type == INST_END_CLASS) printf("Instance END class\n");
	else if (inst->type == INST_END_FUNCTION) printf("Instance END func\n");
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