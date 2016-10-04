//TESTVAL3 := TESTVAL1 + TESTVAL2
#include <stdio.h>
#include <stdlib.h>
#include "interpret.h"
#define NUM_INST 2

int main(int argc, char const *argv[])
{
	TInstr* Inst = malloc(NUM_INST * sizeof(*Inst));
	Inst[0].action = I_ADD;
	Inst[0].op1 = &TESTVAL1;
	Inst[0].op2 = &TESTVAL2;
	Inst[1].action = I_ASS;
	int temp = *(Inst[0].op1) + *(Inst[0].op2);
	Inst[1].op1 = malloc(sizeof(temp));
	Inst[1].op2 = &temp;

	int ret; //return value, result
	for (int i = 0; i < NUM_INST; i++) {
		printf("STEP %d:\n", i);
		ret = Interpret(Inst[i]);
	}
	printf("Result %d\n", ret);
	return ret;
}

int Interpret(TInstr inst) {
	switch (inst.action) {
		case I_STOP:
			printf("Stopping...\n");
			return 0;
		case I_ASS:
			printf("Assining...\n");
			//And store the val in memory/stack so it could be accessed easily
			return *(inst.op2); //var should be named after Inst[i].op1
		case I_ADD:
			printf("Add...\n");
			return *(inst.op1) + *(inst.op2);
		case I_SUB:
			printf("Sub...\n");
			return *(inst.op1) - *(inst.op2);
	}
	return -1;
}