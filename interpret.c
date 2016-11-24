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
		if(s->inst[i]->type == INST_INSTRUCTION && !inFunc) executeInstr(s->inst[i]->instr, s, i); 

	}
	//osetreni chybnosti run() v Mainu
	printf("STEP 2 - validating existance of run() in Main\n");
	if (run == NIL_VALUE || endRun == NIL_VALUE) {
		printf("No run() in Main found, aborting execution\n");
		return INT_ERROR;
	}
	//exekuce instrukci funkci run()
	printf("STEP 3 - executing run() instructions\n");
	for(int i = run; i > endRun; ) {
		//exekuce instrukci
		if(s->inst[i]->type == INST_INSTRUCTION) {
			i = executeInstr(s->inst[i]->instr, s, i);				
		} 
		else {
			i--;
		}
	}
	return 0;
}

//Exekuce jednotlive instrukci
int executeInstr(tInstruction *i, tInstrStack *s, int j) {
	printInstr(i);
	if (i->op == INSTR_WHILE) {
			printf("INSTR_WHILE\n");
		}	
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
		switch (data->type) {
			case INT:
				if (tmp->type == INT)
					data->value.integer = tmp->value.integer;
				else
					printf("Assignment between %d and %d\n", data->type, tmp->type);
				break;
			case DOUBLE:
				if (tmp->type == DOUBLE)
					data->value.real = tmp->value.real;
				else if (tmp->type == INT)
					data->value.real = (double)tmp->value.integer;
				else printf("Undefined type of data\n");
				break;
			case STRING:
				data->value.str = tmp->value.str;
				break;
			case BOOLEAN:
				data->value.boolean = tmp->value.boolean;
				break;
			default:
				printf("Undefined type of variable\n");
				break;
		}
		
		dStackPrint(&ds);
	}
	else if(i->op == PLUS) {
		tData *tma, *tmb, sum;
		tma = dStackPop(&ds);
		tmb = dStackPop(&ds);
		if (tma->type == tmb->type){
			sum.type = tma->type;
			if (tma->type == INT) sum.value.integer = tma->value.integer + tmb->value.integer;
			if (tma->type == DOUBLE) sum.value.real = tma->value.real + tmb->value.real;
			//if (tma->type == STRING) sum.value.str = 
		}
		else if (tma->type == DOUBLE && tmb->type == INT)
		{
			sum.type = tma->type;
			sum.value.real = tma->value.real + (double)tmb->value.integer;
		}
		else if (tma->type == INT && tmb->type == DOUBLE)
		{
			sum.type = tmb->type;
			sum.value.real = (double)tma->value.integer + tmb->value.real;
		}
		dStackPush(&ds, &sum);
		dStackPrint(&ds);
	}
	else if(i->op == MINUS) {
		tData *tma, *tmb, sum;
		tma = dStackPop(&ds);
		tmb = dStackPop(&ds);
		if (tma->type == tmb->type){
			sum.type = tma->type;
			if (tma->type == INT) sum.value.integer = tmb->value.integer - tma->value.integer;
			if (tma->type == DOUBLE) sum.value.real = tmb->value.real - tma->value.real;
		}
		else if (tma->type == DOUBLE && tmb->type == INT)
		{
			sum.type = tma->type;
			sum.value.real = tmb->value.real - (double)tma->value.integer;
		}
		else if (tma->type == INT && tmb->type == DOUBLE)
		{
			sum.type = tmb->type;
			sum.value.real = (double)tmb->value.integer - tma->value.real;
		}
		dStackPush(&ds, &sum);
		dStackPrint(&ds);
	}
	else if(i->op == MUL) {
		tData *tma, *tmb, sum;
		tma = dStackPop(&ds);
		tmb = dStackPop(&ds);
		if (tma->type == tmb->type){
			sum.type = tma->type;
			if (tma->type == INT) sum.value.integer = tma->value.integer*tmb->value.integer;
			if (tma->type == DOUBLE) sum.value.real = tma->value.real*tmb->value.real;
		}
		else if (tma->type == DOUBLE && tmb->type == INT)
		{
			sum.type = tma->type;
			sum.value.real = tma->value.real*(double)tmb->value.integer;
		}
		else if (tma->type == INT && tmb->type == DOUBLE)
		{
			sum.type = tmb->type;
			sum.value.real = (double)tma->value.integer*tmb->value.real;
		}
		dStackPush(&ds, &sum);
		dStackPrint(&ds);
	}
	else if(i->op == COMPARISON) {
		tData *tma, *tmb, com;
		tma = dStackPop(&ds);
		tmb = dStackPop(&ds);
		com.type = BOOLEAN;
		switch (tma->type) {
			case INT:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.integer == tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.integer == tmb->value.real);
						break;
				}
				break;
			case DOUBLE:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.real == tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.real == tmb->value.real);
						break;
				}
				break;
		}
		dStackPush(&ds, &com);
		dStackPrint(&ds);
	}
	else if (i->op == LESS) {
		tData *tma, *tmb, com;
		tmb = dStackPop(&ds);
		tma = dStackPop(&ds);
		com.type = BOOLEAN;
		switch (tma->type) {
			case INT:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.integer < tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.integer < tmb->value.real);
						break;
				}
				break;
			case DOUBLE:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.real < tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.real < tmb->value.real);
						break;
				}
				break;
		}
		dStackPush(&ds, &com);
		dStackPrint(&ds);
	}
	else if (i->op == GREATER) {
		tData *tma, *tmb, com;
		tmb = dStackPop(&ds);
		tma = dStackPop(&ds);
		com.type = BOOLEAN;
		switch (tma->type) {
			case INT:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.integer > tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.integer > tmb->value.real);
						break;
				}
				break;
			case DOUBLE:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.real > tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.real > tmb->value.real);
						break;
				}
				break;
		}
		dStackPush(&ds, &com);
		dStackPrint(&ds);
	}
	else if (i->op == LOEQ) {
		tData *tma, *tmb, com;
		tmb = dStackPop(&ds);
		tma = dStackPop(&ds);
		com.type = BOOLEAN;
		switch (tma->type) {
			case INT:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.integer <= tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.integer <= tmb->value.real);
						break;
				}
				break;
			case DOUBLE:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.real <= tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.real <= tmb->value.real);
						break;
				}
				break;
		}
		dStackPush(&ds, &com);
		dStackPrint(&ds);
	}
	else if (i->op == GOEQ) {
		tData *tma, *tmb, com;
		tmb = dStackPop(&ds);
		tma = dStackPop(&ds);
		com.type = BOOLEAN;
		switch (tma->type) {
			case INT:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.integer >= tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.integer >= tmb->value.real);
						break;
				}
				break;
			case DOUBLE:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.real >= tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.real >= tmb->value.real);
						break;
				}
				break;
		}
		dStackPush(&ds, &com);
		dStackPrint(&ds);
	}
	else if(i->op == NEQ) {
		tData *tma, *tmb, com;
		tma = dStackPop(&ds);
		tmb = dStackPop(&ds);
		com.type = BOOLEAN;
		switch (tma->type) {
			case INT:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.integer != tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.integer != tmb->value.real);
						break;
				}
				break;
			case DOUBLE:
				switch (tmb->type) {
					case INT:
						com.value.boolean = (tma->value.real != tmb->value.integer);
						break;
					case DOUBLE:
						com.value.boolean = (tma->value.real != tmb->value.real);
						break;
				}
				break;
		}
		dStackPush(&ds, &com);
		dStackPrint(&ds);
	}
	else if(i->op == AND) {
		tData *tma, *tmb, com;
		bool a, b;
		tma = dStackPop(&ds);
		tmb = dStackPop(&ds);
		switch (tma->type) {
			case INT:
				a = (bool)tma->value.integer;
				break;
			case DOUBLE:
				a = (bool)tma->value.integer;
				break;
			case BOOLEAN:
				a = tma->value.boolean;
				break;
		}
		switch (tmb->type) {
			case INT:
				b = (bool)tmb->value.integer;
				break;
			case DOUBLE:
				b = (bool)tmb->value.integer;
				break;
			case BOOLEAN:
				b = tmb->value.boolean;
				break;
		}
		com.type = BOOLEAN;
		com.value.boolean = a && b;
		dStackPush(&ds, &com);
		dStackPrint(&ds);
	}
	else if(i->op == OR) {
		tData *tma, *tmb, com;
		bool a, b;
		tma = dStackPop(&ds);
		tmb = dStackPop(&ds);
		switch (tma->type) {
			case INT:
				a = (bool)tma->value.integer;
				break;
			case DOUBLE:
				a = (bool)tma->value.integer;
				break;
			case BOOLEAN:
				a = tma->value.boolean;
				break;
		}
		switch (tmb->type) {
			case INT:
				b = (bool)tmb->value.integer;
				break;
			case DOUBLE:
				b = (bool)tmb->value.integer;
				break;
			case BOOLEAN:
				b = tmb->value.boolean;
				break;
		}
		com.type = BOOLEAN;
		com.value.boolean = a || b;
		dStackPush(&ds, &com);
		dStackPrint(&ds);
	}
	else if (i->op == INSTR_IF) {
		tData *con;
		int *endif = (int *)i->addr1, *begel = (int *)i->addr2, *endel = (int *)i->addr3;
		con = dStackPop(&ds);
		if (con->value.boolean)
		{
			j--;
			while ( j > *endif ) {
				j = executeInstr(s->inst[j]->instr, s, j); 
			}
			if (begel != NULL && endel != NULL)
				return (*endel-1);
		}
		else if (begel != NULL && endel != NULL) {
			j = *begel;
			while ( j > *endel ) { 
				j = executeInstr(s->inst[j]->instr, s, j); 
			}
		}
		else if (endif != NULL) {
			return (*endif-1);
		}
	}
	else if (i->op == INSTR_WHILE) {
		tData *con;
		int *cond = (int *)i->addr1, *endwhile = (int *)i->addr2;
		con = dStackPop(&ds);
		printf("%s\n", con->value.boolean ?"true":"false");
		int begwhile = j;
		j--;
		while (con->value.boolean)
		{
			j = begwhile - 1;
			while ( j != *endwhile ) {
				j = executeInstr(s->inst[j]->instr, s, j); 
			}
			j = *cond;
			while ( j != begwhile ) {
				j = executeInstr(s->inst[j]->instr, s, j); 
			}
			con = dStackPop(&ds);
			printf("%s\n", con->value.boolean ?"true":"false");
		}
		return (*endwhile-1);
	}
	else if (i->op == INSTR_CALL_FUNC) {
		int *func = (int *)i->addr3;
		int jnasl = j-1;
		j = *func;
		while( s->inst[j]->type != INST_END_FUNCTION ){
			j = executeInstr(s->inst[j]->instr, s, j);
			if (s->inst[j]->type == INST_END_FUNCTION){
				dStackPrint(&ds);
			}
		
		}
		//printf("dSteck after function cykle\n");
		dStackPrint(&ds);
		return jnasl;
	}
	else if (i->op == INSTR_ASS_ARG) {
		tData *arg = (tData *)i->addr3;
		tData *tmp;
		tmp = dStackPop(&ds);
		switch (arg->type) {
			case INT:
				arg->value.integer = tmp->value.integer;
				break;
			case DOUBLE:
				if (tmp->type == DOUBLE)
					arg->value.real = tmp->value.real;
				else if (tmp->type == INT)
					arg->value.real = (double)tmp->value.integer;
				break;
			case STRING:
				arg->value.str = tmp->value.str;
				break;
			case BOOLEAN:
				arg->value.boolean = tmp->value.boolean;
				break;
			default:
				printf("undefined type of arg in INSTR_ASS_ARG in %d instruction\n", j);
				break;
			}
		dStackPrint(&ds);
	}
	else if (i->op == INSTR_PRINT) {
		tData *str;
		str = dStackPop(&ds);
		switch (str->type)
		{
			case INT:
		}

	}
	else if (i->op == INSTR_R_INT) {
		tData data; 
		data.type = INT;
		data.value.integer = read_int();

		dStackPush(&ds, &data);

	}
	else if (i->op == INSTR_R_STRING) {
		tData data; 
		data.type = STRING;
		data.value.str = read_string();

		dStackPush(&ds, &data);

	}
	else if (i->op == INSTR_R_DOUBLE) {
		tData data; 
		data.type = DOUBLE;
		data.value.real = read_double();

		dStackPush(&ds, &data);

	}
	else if (i->op == INSTR_LENGTH) {
		tData *str = dStackPop(&ds);
		tData len;

		len.type = INT;
		len.value.integer = length(str->value.string);

		dStackPush(&ds, &len);
	}
	else if (i->op == INSTR_SUBSTR) {
		tData *n = dStackPop(&ds),
			*i = dStackPop(&ds),
			*str = dStackPop(&ds);
		tData retstr;

		retstr.type = STRING;
		retstr.value.str = substr(str->value.string, i.value.integer, n.value.integer);

		dStackPush(&ds, &retstr);
	}
	else if (i->op == INSTR_COMPARE) {
		tData *s2 = dStackPop(&ds),
			*s1 = dStackPop(&ds);
		tData i;

		i.type = INT;
		i.value.integer = compare(s1->value.string, s2->value.string);

		dStackPush(&ds, &i);
	}
	else if (i->op == INSTR_SORT) {
		tData *str = dStackPop(&ds);
		tData retstr;

		retstr.type = STRING;
		retstr.value.string = sort(str->value.string);

		dStackPush(&ds, &retstr);
	}

	return j-1;
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
				printf("INSERT %g\n", data->value.real);
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
		switch (data->type) {
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
	else if (i->op == MINUS) {
			printf("MINUS\n");
		}
	else if (i->op == INSTR_IF) {
			printf("INSTR_IF\n");
		}
	else if (i->op == INSTR_BEG_COND) {
			printf("INSTR_BEG_COND\n");
		}
	else if (i->op == INSTR_END_BLCK) {
			printf("INSTR_END_BLCK\n");
		}
	else if (i->op == COMPARISON) {
			printf("COMPARISON\n");
		}
	else if (i->op == LESS) {
			printf("LESS\n");
		}
	else if (i->op == INSTR_WHILE) {
		printf("WHILE\n");
	}
	else if (i->op == INCREMENT) {
			printf("INCREMENT\n");
		}
	else if (i->op == DECREMENT) {
		printf("DECREMENT\n");
	}
	else if (i->op == INSTR_CALL_FUNC) {
			printf("INSTR_CALL_FUNC\n");
		}
	else if (i->op == INSTR_ASS_ARG) {
			printf("INSTR_ASS_ARG\n");
		}
	else {
		printf("UNKNOWN INSTRUCTION\n");
	}
}
/**/

//inicializace zasobniku DStack
void dStackInit(tDStack *s) {
	if (s != NULL) {
		s->top = NIL_VALUE;
		s->arr = malloc(MAX_STACK * sizeof(tData *));
		if (s->arr == NULL) {
			printf("Allocation error of stack\n");
		}
	}
	else {
		printf("Pointer on stack is undefined\n");
	}
}

//Vypis zasobniku DStack
void dStackPrint(tDStack *s) {
	printf("---------------------------------------------------\n");
	for(int i = s->top; i > NIL_VALUE; i--) {
		switch (s->arr[i]->type) {
			case INT:
				printf("N%d: type is %d, values is %d\n", i, s->arr[i]->type, s->arr[i]->value.integer);
				break;
			case DOUBLE:
				printf("N%d: type is %d, values is %g\n", i, s->arr[i]->type, s->arr[i]->value.real);
				break;
			case STRING:
				printf("N%d: type is %d, values is %s\n", i, s->arr[i]->type, s->arr[i]->value.str);
				break;
			case VOID:
				printf("N%d: type is %d, values is void\n", i, s->arr[i]->type);
				break;
			case BOOLEAN:
				printf("N%d: type is %d, values is %d (bool) \n", i, s->arr[i]->type, s->arr[i]->value.boolean);
				break;
			default:
				printf("???\n");
				break;
		}
	}
	printf("---------------------------------------------------\n");
}

//Implementace instrukci push pro zasobnik DStack
void dStackPush(tDStack *s, tData *data) {
	if (s != NULL) {
		if (dStackIsFull(&ds)) {
			printf("DataStack is full\n");
		}
		else {
			s->top++;
			s->arr[s->top] = data;
		}
	}
	else {
		printf("Pointer on stack is undefined\n");
	}
}

//Implementace instrukci pop pro zasobnik DStack
tData *dStackPop(tDStack *s) {
	tData *tmp;
	if (s != NULL) {
		if (dStackIsEmpty(&ds)) {
			printf("Pop from empty dataStack\n");
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
		printf("Pointer on stack is undefined\n");
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
		printf("Pointer on stack is undefined\n");
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
		printf("Pointer on stack is undefined\n");
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
