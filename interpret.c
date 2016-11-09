#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "str.h"
#include "test_inst.h"
#include "test_table.h"
#include "test_scanner.h"
#include "interpret.h"

tDStack ds;

int execute(tInstrStack *s) {
	dStackInit(&ds);
	int run = -1;	//misto vyskytu prvni instrukci funkce run() tridy Main
	//pomocne promenne
	bool inMain = false;
	for(int i = s->top; i > -1; i--) {
		//nalezeni instrukci tridy Main
		if(s->inst[i]->type == INST_CLASS && !strCmpConstStr(s->inst[i]->name, "Main")) {
			inMain = true;	//nastaveni pomocne promenne
		}
		//exekuce instrukci Main
		if(s->inst[i]->type == INST_INSTRUCTION && inMain) {
			executeInst(s->inst[i]);
		}
		//nalezeni instrukci run()
		if(s->inst[i]->type == INST_FUNCTION && !strCmpConstStr(s->inst[i]->name, "run") && inMain) {
			run = i;
			break;
		}
	}
	//osetreni chyby NORUNF
	if (run == -1) {
		//err
		printf("Could not find run in Main, aborting execution\n");
		return -1;
	}

	//exekuce instrukci run()
	for(int i = run; s->inst[i]->type != INST_END_FUNCTION; i--) {
		executeInst(s->inst[i]);
	}
	return 0;
}

//Exekuce jednotlive instrukci
void executeInst(tInstance *inst) {
	if (inst->type == INST_INSTRUCTION) {
		printf("Executing instruction with with opcode [%d]\n", inst->instr->op);
		//INSERT - push hodnoty do DStack
		if(inst->instr->op == INSTR_INSERT) {
			dStackPush(&ds, (tData *)(inst->instr->addr1));
			dStackPrint(&ds);
		}
		//ASSIGNMENT - prirazeni hodnoty z vrcholu zasobniku DStack do promenne na adrese addr3
		if(inst->instr->op == ASSIGNMENT) {
			inst->instr->addr3 = dStackPop(&ds);
			dStackPrint(&ds);
			//print
			if (((tData *)inst->instr->addr3)->type == INT) {
				printf("Addr3 %d\n", ((tData *)inst->instr->addr3)->value);
			}
			else if (((tData *)inst->instr->addr3)->type == STRING) {
				printf("Addr3 %s\n", ((tData *)inst->instr->addr3)->value);
			}
			/////////////////////
		}
		if(inst->instr->op == PLUS) {
			printf("Executing plus\n");
		}
	}
	//----------------------------------------------------------------------------------------
	else if (inst->type == INST_CLASS) printf("Instance class, name [%s]\n", strGetStr(inst->name));
	else if (inst->type == INST_FUNCTION) printf("Instance func, name [%s]\n", strGetStr(inst->name));
	else if (inst->type == INST_END_CLASS) printf("Instance END class\n");
	else if (inst->type == INST_END_FUNCTION) printf("Instance END func\n");
}

//inicializace zasobniku DStack
void dStackInit(tDStack *s) {
	if (s != NULL) {
		s->top = -1;
		s->arr = malloc(MAX_STACK * sizeof(tData *));
	}
	else {
		//err
	}
}

//Vypis zasobniku DStack
void dStackPrint(tDStack *s) {
	printf("---------------------------------------------------\n");
	for(int i = s->top; i > -1; i--) {
		printf("N%d: type is %d, values is %d\n", i, s->arr[i]->type, s->arr[i]->value);
	}
	printf("---------------------------------------------------\n");
}

//Implementace instrukci push pro zasobnik DStack
void dStackPush(tDStack *s, tData *data) {
	if (s != NULL) {
		if (dStackIsFull(&ds)) {
			//err/realloc
		}
		else {
			s->top++;
			s->arr[s->top] = data;
		}
	}
	else {
		//err
	}
}

//Implementace instrukci pop pro zasobnik DStack
tData *dStackPop(tDStack *s) {
	tData *tmp;
	if (s != NULL) {
		if (dStackIsEmpty(&ds)) {
			//err/realloc
			return NULL;
		}
		else {
			tmp = s->arr[s->top];
			s->arr[s->top] = NULL; 
			s->top--;
			return tmp;
		}
	}
	else {
		//err
		return NULL;
	}
}

bool dStackIsFull(tDStack *s) {
	return s->top >= (int)MAX_STACK;
}

bool dStackIsEmpty(tDStack *s) {
	return s->top == -1;
}
