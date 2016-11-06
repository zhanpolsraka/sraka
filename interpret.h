#ifndef _INTERPRET_H_
#define _INTERPRET_H_

typedef struct {
	int size;
	tData *top;
}tDStack;

void dStackInit(tDStack *s);
void dStackPush(tDStack *s, tData *data);
int execute(tInstrStack *s);

#endif
