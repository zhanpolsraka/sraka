#ifndef _ERROR_H_
#define _ERROR_H_

/*	Typy chyb	*/
typedef enum{

	LEX_ERROR,		// lexikalni chyba		(0)
	SYN_ERROR,		// syntakticka chyba	(1)
	SEM_ERROR,		// semanticka chyba		(2)
	INT_ERROR,		// chyba interpretu		(3)
	PR_ERROR,		// chyba programu		(4)
	ALLOC_ERROR		// chyba alokaci		(5)

}tErrType;

void printErr(tErrType t_err, char * str, int line);

#endif