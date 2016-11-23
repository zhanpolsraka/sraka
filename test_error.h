#ifndef _ERROR_H_
#define _ERROR_H_

/*	Typy chyb	*/
typedef enum{

	LEX_ERROR,		// lexikalni chyba		(0)
	SYN_ERROR,		// syntakticka chyba	(1)
	SEM_ERROR,		// semanticka chyba		(2)
	INT_ERROR,		// chyba interpretu		(3)
	PR_ERROR,		// chyba programu		(4)
	ALLOC_ERROR,	// chyba alokaci		(5)

	INPUT_NUM_ERROR, // behova chyba pri nacitani ciselne hodnoty ze vstupu (7)
	OTHER_ERROR,	 // ostatni behove chyby (10)

	UNK_CLASS_BEG,
	MAIN_EXIST,
	FUNC_EXIST,
	CLASS_EXIST,
	OPEN_FILE

}tErrType;

typedef enum{

	ALL_STRUCT,		// chyba alokaci pameti pro strukturu
	CLASS_KEY,		// chyba zacatku tridy
	ID_KEY,			// chyba identifikatoru
	TREE_CLASS,		// trida s takovym id uz existuje
	TREE_ERROR,		// chyba v zpracovani binarniho stromu
	TREE_FUNC,		// funkce s takovym id uz existuje
	TYPE_KEY,		// ocekavame jiny typ
	UNK_EXPR,		// neznamy vyraz
	UNK_LEX,		// neznamy lexem
	FILE_ERROR,		// chyba otevreni souboru
	CALL_FUNC_ARG,	// chyba v argumentech volane funkci
	UNDEF_VAR,
	UNDEF_CLASS,
	UNDEF_FUNC,
	ILLEGAL_OP,
	RET_VALUE

}tErrReas;

void throw_err(tErrType err_type, int reason);

#endif
