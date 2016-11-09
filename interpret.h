#ifndef _INTERPRET_H_
#define _INTERPRET_H_
#define MAX_STACK 1024

typedef struct {
	//tData *arr[MAX_STACK];
	tData **arr;
	int top;
}tDStack;

void dStackInit(tDStack *s);
void dStackPush(tDStack *s, tData *data);
tData *dStackPop(tDStack *s);
void dStackPrint(tDStack *s);
bool dStackIsFull(tDStack *s);
bool dStackIsEmpty(tDStack *s);
void executeInst(tInstance *inst);
int execute(tInstrStack *s);

#endif
