#ifndef _TOKEN_H_
#define _TOKEN_H_

extern int line;

/*	Typy lexemu	*/
typedef enum{

    IDENTIFIER, // identifikator        (0)
    PLUS,       // plus                 (1)
    MINUS,      // minus                (2)
    MUL,        // nasobeni             (3)
    DIV,        // deleni               (4)
    SEMICOLON,  // strednik             (5)

    VALUE,

    L_PAR,      // leva zavorka         (6)
    R_PAR,      // prava zavorka        (7)

    COMPARISON, // porovnani            (8)
    LESS,       // mensi                (9)
    GREATER,    // vetsi                (10)
    LOEQ,       // mensi nebo ravno     (11)
    GOEQ,       // vetsi nebo ravno     (12)
    NEQ,        // neravno              (13)
    NOT,        // negace               (14)

    COMMA,      // carka                (15)

    L_VIN,      // leva hranata zavorka (16)

    OR, //(17)
    AND, // (18)

    R_VIN,      // prava hranata zavorka(17)
    KEYWORD,    // klicove slovo        (18)
    INT,        // typ int              (19)
    DOUBLE,     // typ double           (20)
    STRING,     // typ retezec          (21)
    VOID,       // prazdny typ          (22)
    BOOLEAN,    // buleovsky typ        (23)
    SYMB,       //                      (24)

    ASSIGNMENT, // prirazeni            (25)
    INCREMENT,  // incrementace         (26)
    DECREMENT,  // decrementace         (27)

    POINT,      // tecka                (28)
    ESCSEQ,     // escape sekvence      (29)

    SPACE,      // mezera               (30)
    T_EOF		// konec souboru		(32)

}tType;

/*	Struktura tokenu	*/
typedef struct{

	string attr;
	tType type;

} Token;

/* Pomocna struktura pro ukladani informace o promenne/funkci   */
typedef struct{

    bool st_static;
    tType type;
    string identifier;

}tHelpVar;

void open_source(const char *file_name);
void close_source();
void get_token(Token *token);
void init_token(Token *token);
void free_token(Token *token);
void go_back(Token *token);

#endif
