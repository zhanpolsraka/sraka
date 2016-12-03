#ifndef _ERROR_H_
#define _ERROR_H_

/*	Typy chyb	*/
typedef enum{

	LEX_ERROR = 1, //chyba v programu v rámci lexikální analýzy
					//(chybná struktura aktuálního lexému)
	SYN_ERROR, //2- chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu)
	SEM_ERROR, //3- sémantická chyba v programu – nedefinovaná trída/funkce/promenná,
				//pokus o redefinici trídy/funkce/promenné, atd
	SEM_TYPE_ERROR, //4- sémantická chyba typové kompatibility v aritmetických,
					//retezcových a relacních výrazech, príp. špatný pocet ci
					//typ parametru u volání funkce.
	SEM_OTHER = 6,  // ostatní sémantické chyby.
	READ_ERROR,		//7 - behová chyba pri nacítání císelné hodnoty ze vstupu
	UNINIT_ERROR,	//8 - behová chyba pri práci s neinicializovanou promennou.
	DIV_ERROR,		//9 - behová chyba delení nulou
	OTHER_ERROR,	//10 - ostatní behové chyby.
	INT_ERROR = 99	// - interní chyba interpretu tj. neovlivnená vstupním programem
					//(napr. chyba alokace pameti, chyba pri otvírání souboru s rídicím
					//programem, špatné parametry príkazové rádky atd.).
}tErrType;

typedef enum{

	ALL_STRUCT,		// chyba alokaci pameti pro strukturu
	UNK_LEX,		// neznamy lexem
	CALL_FUNC_ARG,	// chyba v argumentech volane funkci
	TYPE_KEY,		// ocekavame jiny typ
	UNK_EXPR,		// neznamy vyraz
	CLASS_EXIST,
	FUNC_EXIST,
	VAR_EXIST,
	EXPEC_TOKEN,
	OTHER_TYPE,
	UNINIT_VAR,
	UNDEF_VAR,
	UNDEF_CLASS,
	UNDEF_FUNC,
	ILLEGAL_OP,
	RET_VALUE

}tErrReas;

void throw_err(tErrType err_type, int reason, char *addition);
#endif
