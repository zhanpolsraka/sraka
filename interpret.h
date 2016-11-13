#ifndef _INTERPRET_H_
#define _INTERPRET_H_
#define MAX_STACK 1024
#define NIL_VALUE -1

typedef struct {
	//tData *arr[MAX_STACK];
	tData **arr;
	int top;
}tDStack;

void printInstr(tInstruction *i); //helper function - remove when interpret's done!!!
void dStackInit(tDStack *s);
void dStackPush(tDStack *s, tData *data);
tData *dStackPop(tDStack *s);
tData *dStackTop(tDStack *s);
void dStackPrint(tDStack *s);
bool dStackIsFull(tDStack *s);
bool dStackIsEmpty(tDStack *s);
void dStackReverse(tDStack *s);
void executeInstr(tInstruction *i);
int execute(tInstrStack *s);

#endif
