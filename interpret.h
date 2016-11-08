#ifndef _INTERPRET_H_
#define _INTERPRET_H_
#define MAX_STACK 100

typedef struct {
	tData *arr[MAX_STACK];
	int top;
}tDStack;

void dStackInit(tDStack *s);
void dStackPush(tDStack *s, tData *data);
void dStackPrint(tDStack *s);
void executeInst(tInstance *inst);
int execute(tInstrStack *s);

#endif
