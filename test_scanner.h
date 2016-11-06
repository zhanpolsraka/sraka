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

    L_VIN,      // leva hranata zavorka (25)
    R_VIN,      // prava hranata zavorka(26)
    KEYWORD,    // klicove slovo        (13)
    INT,        // typ int              (14)
    DOUBLE,     // typ double           (15)
    STRING,     // typ retezec          (16)
    VOID,       // prazdny typ          (17)
    BOOLEAN,    // buleovsky typ        (18)

    ASSIGNMENT, // prirazeni            (19)
    INCREMENT,  // incrementace         (20)
    DECREMENT,  // decrementace         (21)

    POINT,      // tecka                (22)
    ESCSEQ,     // escape sekvence      (24)

    SPACE,      // mezera               (27)
    NEW_LINE,   // novy radek           (28)
    T_EOF		// konec souboru		(29)

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
bool get_token(Token *token);
bool init_token(Token *token);
void free_token(Token *token);
void go_back(Token *token);

#endif
