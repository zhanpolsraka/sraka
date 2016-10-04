#define I_STOP 0 
#define I_ASS 1 
#define I_ADD 7  
#define I_SUB 8
static int TESTVAL1 = 1;
static int TESTVAL2 = 2;

typedef struct 
{
	int action;
	int *op1;
	int *op2;
}TInstr;

int Interpret(TInstr inst);