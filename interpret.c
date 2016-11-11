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
		/*Delete this later - for debugging purposes*/
		if (s->inst[i]->type == INST_CLASS) printf("Instance class, name [%s]\n", strGetStr(s->inst[i]->name));
		else if (s->inst[i]->type == INST_FUNCTION) printf("Instance func, name [%s]\n", strGetStr(s->inst[i]->name));
		else if (s->inst[i]->type == INST_END_CLASS) printf("Instance END class\n");
		else if (s->inst[i]->type == INST_END_FUNCTION) printf("Instance END func\n");
		/**/

		//exekuce instrukci INSERT
		if(s->inst[i]->type == INST_INSTRUCTION && s->inst[i]->instr->op == INSTR_INSERT) {
			executeInstr(s->inst[i]->instr);
		}
		//nalezeni instrukci tridy Main
		if(s->inst[i]->type == INST_CLASS && !strCmpConstStr(s->inst[i]->name, "Main")) {
			inMain = true;	//nastaveni pomocne promenne
		}
		//nalezeni instrukci run()
		if(s->inst[i]->type == INST_FUNCTION && !strCmpConstStr(s->inst[i]->name, "run") && inMain) {
			run = i;
		}
	}
	//osetreni chyby NORUNF
	if (run == -1) {
		//err
		printf("Could not find run in Main, aborting execution\n");
		return -1;
	}
	//exekuce instrukci run()
	dStackReverse(&ds);
	for(int i = run; s->inst[i]->type != INST_END_FUNCTION; i--) {
		if (s->inst[i]->type == INST_INSTRUCTION && s->inst[i]->instr->op != INSTR_INSERT) executeInstr(s->inst[i]->instr);
	}

	//exekuce ostatnich instrukci - TODO
	return 0;
}

//Exekuce jednotlive instrukci
void executeInstr(tInstruction *i) {
	printInstr(i);	
	//INSERT - push hodnoty do DStack
	if(i->op == INSTR_INSERT) {
		dStackPush(&ds, (tData *)(i->addr1));
		dStackPrint(&ds);
	}
	//ASSIGNMENT - prirazeni hodnoty z vrcholu zasobniku DStack do promenne na adrese addr3
	else if(i->op == ASSIGNMENT) {
		i->addr3 = dStackPop(&ds);
		dStackPrint(&ds);
	}
}

/*helper function - Delete this later - for debugging purposes*/
void printInstr(tInstruction *i) {
	//------------------------------------INSERT
	if (i->op == INSTR_INSERT) {
		switch (((tData *)(i->addr1))->type) {
			case INT:
				printf("INSERT %d\n", ((tData *)(i->addr1))->value);
				break;
			case DOUBLE:
				printf("INSERT %f\n", ((tData *)(i->addr1))->value);
				break;
			case STRING:
				printf("INSERT %s\n", ((tData *)(i->addr1))->value);
				break;
			case VOID:
				break;
			case BOOLEAN:
				printf("INSERT %d (boolean)\n", ((tData *)(i->addr1))->value);
				break;
			default:
				printf("INSERT Unknown type variable\n");
				break;
		}
	}
	//------------------------------------ASSIGNMENT
	else if (i->op == ASSIGNMENT) {
		switch ((dStackTop(&ds))->type) {
			case INT:
				printf("ASSIGN %d to %p\n", (dStackTop(&ds))->value.integer, i->addr3);
				break;
			case DOUBLE:
				printf("ASSIGN %f to %p\n", (dStackTop(&ds))->value.real, i->addr3);
				break;
			case STRING:
				printf("ASSIGN %s to %p\n", (dStackTop(&ds))->value.str, i->addr3);
				break;
			case VOID:
				break;
			case BOOLEAN:
				printf("ASSIGN %d (boolean) to %p\n", (dStackTop(&ds))->value.boolean, i->addr3);
				break;
			default:
				printf("ASSIGN Unknown type variable to %p\n", i->addr3);
				break;
		}
	}
}
/**/

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
			//err
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

//Vraci obsah polozky top (posledni, "horni" ulozena polozka) zasobniku DStack
//(podobne fce dStackPop, ale nezmeni obsah zasobniku)
tData *dStackTop(tDStack *s) {
	if (s != NULL) {
		return dStackIsEmpty(s) ? NULL : s->arr[s->top];
	}
	else {
		//err
		return NULL;
	}
}

//Vraci true jestli DStack je plny
bool dStackIsFull(tDStack *s) {
	return s->top >= (int)MAX_STACK;
}

//Vraci true jestli DStack je prazdny
bool dStackIsEmpty(tDStack *s) {
	return s->top == -1;
}

//Vraci pocet polozek v zasobniku DStack
int dStackSize(tDStack *s) {
	return s->top + 1;
}
//Prehodi polozky DStack mezi sebou
void dStackReverse(tDStack *s) {
	if (s == NULL) {
		//err
	}
	else if (!dStackIsEmpty(s)) {
		tData *tmp;
		int j = 0;
		for(int i = s->top; i > -1; i--) {
			tmp = s->arr[i];
			s->arr[i] = s->arr[j];
			s->arr[j] = tmp;
			j++;
		}
	}
}
