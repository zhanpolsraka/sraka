#ifndef _TOKEN_H_
#define _TOKEN_H_

extern int line;

/*	Typy lexemu	*/
typedef enum{

   	KEYWORD,    // klicove slovo 		(0)
    DATA_TYPE,  // datovy typ           (1)
    IDENTIFIER,	// identifikator 		(2)
    INT,		// typ int 				(3)
    DOUBLE,		// typ double  			(4)
    STRING,		// typ retezec 			(5)
    SYMB,		// typ symbol			(6)
    POINT,		// tecka				(7)
    COMMA,		// carka				(8)
    ESCSEQ,		// escape sekvence 		(9)
    COMPARISON,	// porovnani 			(10)
    ASSIGNMENT,	// prirazeni 			(11)
    PLUS,		// plus 	 			(12)
    MINUS,		// minus 				(13)
    MUL,		// nasobeni				(14)
    DIV,		// deleni				(15)
    LESS,		// mensi				(16)
    GREATER,	// vetsi				(17)
    LOEQ,		// mensi nebo ravno		(18)
    BOEQ,		// vetsi nebo ravno		(19)
    NOT,		// negace				(20)
    NEQ,		// nonekvivalence		(21)
    INCREMENT,	// incrementace			(22)
    DECREMENT,	// decrementace			(23)
    L_PAR,		// leva zavorka			(24)
    R_PAR,		// prava zavorka		(25)
    L_VIN,		// leva hranata zavorka (26)
    R_VIN,		// prava hranata zavorka(27)
    SEMICOLON,	// strednik				(28)

    SPACE,      // mezera               (29)
    NEW_LINE,   // novy radek           (30)
    T_EOF		// konec souboru		(31)

}tType;

/*	Struktura tokenu	*/
typedef struct{

	string attr;
	tType type;

} Token;

void set_source(const char *file_name);
bool get_token(Token *token);
bool skip_place(Token *token);
bool skip_space(Token *token);

#endif