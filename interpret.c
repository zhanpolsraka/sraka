#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "str.h"
#include "test_inst.h"
#include "test_table.h"
#include "test_scanner.h"
#include "test_error.h"
#include "interpret.h"

tDStack ds;

int execute(tInstrStack *s) {
	//pomocne flagy
	bool inFunc = false;
	bool inMain = false;
	int run = NIL_VALUE;
	int endRun = NIL_VALUE;
	//inicializace DStack
	dStackInit(&ds);
	//exekuce instrukci mimo funkci a zaroven nalezeni instrukci funkci run()
	for(int i = s->top; i > NIL_VALUE; i--) {
		printf("STEP 1 - non-func instructions\n");
		//nalezeni instrukci tridy Main
		if(s->inst[i]->type == INST_CLASS && !strCmpConstStr(s->inst[i]->name, "Main")) {
			printf("Found Main\n");
			inMain = true;
		}
		//nalezeni konce instrukci tridy Main
		if(s->inst[i]->type == INST_END_CLASS && inMain) {
			printf("Got out of Main\n");
			inMain = false;
		}	
		//nalezeni zacatku instrukci run()
		if(s->inst[i]->type == INST_FUNCTION) {
			printf("Got in function - ignore instructions until I get out\n");
			inFunc = true;
			if (!strCmpConstStr(s->inst[i]->name, "run") && inMain) {
				printf("Found run()\n");
				run  = i;
			}
		}
		//nalezeni konce instrukci run()
		if(s->inst[i]->type == INST_END_FUNCTION) {
			printf("Got out of function\n");
			inFunc = false;
			if (run > NIL_VALUE && endRun == NIL_VALUE && inMain) {
				printf("Got out of run()\n");
				endRun = i;
			}
		}
		//exekuce instrukci mimo funkci
		if(s->inst[i]->type == INST_INSTRUCTION && !inFunc) executeInstr(s->inst[i]->instr); 

	}
	//osetreni chybnosti run() v Mainu
	printf("STEP 2 - validating existance of run() in Main\n");
	if (run == NIL_VALUE || endRun == NIL_VALUE) {
		printf("No run() in Main found, aborting execution\n");
		return INT_ERROR;
	}
	//exekuce instrukci funkci run()
	printf("STEP 3 - executing run() instructions\n");
	for(int i = run; i > endRun; i--) {
		//exekuce instrukci
		if(s->inst[i]->type == INST_INSTRUCTION) executeInstr(s->inst[i]->instr);
	}
	return 0;
}

//Exekuce jednotlive instrukci
void executeInstr(tInstruction *i) {
	printInstr(i);	
	//INSERT - push hodnoty do DStack
	if(i->op == INSTR_INSERT) {
		tData *data = (tData *)i->addr1;
		dStackPush(&ds, data);
		dStackPrint(&ds);
	}
	//ASSIGNMENT - prirazeni hodnoty z vrcholu zasobniku DStack do promenne na adrese addr3
	else if(i->op == ASSIGNMENT) {
		tData *data = (tData *)i->addr3;
		tData *tmp;
		tmp = dStackPop(&ds);
		//TODO: different types, not only integer
		data->value.integer = tmp->value.integer;
		dStackPrint(&ds);
	}
	else if(i->op == PLUS) {
		tData *tma, *tmb, *sum;
		tma = dStackPop(&ds);
		tmb = dStackPop(&ds);
		if (tma->type == tmb->type){
			sum->type = tma->type;
			if (tma->type == INT) sum->value.integer = tma->value.integer + tmb->value.integer;
			if (tma->type == DOUBLE) sum->value.real = tma->value.real + tmb->value.real;
			//if (tma->type == STRING) sum->value.str = tma->value.str + tmb->value.str;
		}
		else if (tma->type == DOUBLE && tmb->type == INT)
		{
			sum->type = tma->type;
			sum->value.real = tma->value.real + (double)tmb->value.integer;
		}
		else if (tma->type == INT && tmb->type == DOUBLE)
		{
			sum->type = tmb->type;
			sum->value.real = (double)tma->value.integer + tmb->value.real;
		}
		dStackPush(&ds, sum);
		dStackPrint(&ds);
	}
	else if(i->op == MINUS) {
		tData *tma, *tmb, *sum;
		tma = dStackPop(&ds);
		tmb = dStackPop(&ds);
		if (tma->type == tmb->type){
			sum->type = tma->type;
			if (tma->type == INT) sum->value.integer = tmb->value.integer - tma->value.integer;
			if (tma->type == DOUBLE) sum->value.real = tmb->value.real - tma->value.real;
		}
		dStackPush(&ds, sum);
		dStackPrint(&ds);
	}
	else if(i->op == MUL) {
		tData *tma, *tmb, *mul;
		tma = dStackPop(&ds);
		tmb = dStackPop(&ds);
		if (tma->type == tmb->type){
			mul->type = tma->type;
			if (tma->type == INT) mul->value.integer = (tma->value.integer)*(tmb->value.integer);
			if (tma->type == DOUBLE) mul->value.real = (tma->value.real)*(tmb->value.real);
		}
		dStackPush(&ds, mul);
		dStackPrint(&ds);
	}
}

/*helper function - Delete this later - for debugging purposes*/
void printInstr(tInstruction *i) {
	//------------------------------------INSERT
	if (i->op == INSTR_INSERT) {
		tData *data = (tData *)i->addr1;
		switch (data->type) {
			case INT:
				printf("INSERT %d\n", data->value.integer);
				break;
			case DOUBLE:
				printf("INSERT %f\n", data->value.real);
				break;
			case STRING:
				printf("INSERT %s\n", data->value.str);
				break;
			case VOID:
				break;
			case BOOLEAN:
				printf("INSERT %d (boolean)\n", data->value.boolean);
				break;
			default:
				printf("INSERT Unknown type variable\n");
				break;
		}
	}
	//------------------------------------ASSIGNMENT
	else if (i->op == ASSIGNMENT) {
		tData *data = dStackTop(&ds);
		switch ((dStackTop(&ds))->type) {
			case INT:
				printf("ASSIGN %d to %p\n", data->value.integer, i->addr3);
				break;
			case DOUBLE:
				printf("ASSIGN %f to %p\n", data->value.real, i->addr3);
				break;
			case STRING:
				printf("ASSIGN %s to %p\n", data->value.str, i->addr3);
				break;
			case VOID:
				break;
			case BOOLEAN:
				printf("ASSIGN %d (boolean) to %p\n", data->value.boolean, i->addr3);
				break;
			default:
				printf("ASSIGN Unknown type variable to %p\n", i->addr3);
				break;
		}

	}
	else if (i->op == PLUS) {
			printf("PLUS\n");
		}
}
/**/

//inicializace zasobniku DStack
void dStackInit(tDStack *s) {
	if (s != NULL) {
		s->top = NIL_VALUE;
		s->arr = malloc(MAX_STACK * sizeof(tData *));
		if (s->arr == NULL) {
			//ALOC_ERR
		}
	}
	else {
		//err
	}
}

//Vypis zasobniku DStack
void dStackPrint(tDStack *s) {
	printf("---------------------------------------------------\n");
	for(int i = s->top; i > NIL_VALUE; i--) {
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
	return s->top == NIL_VALUE;
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
		for(int i = s->top; i > NIL_VALUE; i--) {
			tmp = s->arr[i];
			s->arr[i] = s->arr[j];
			s->arr[j] = tmp;
			j++;
		}
	}
}
