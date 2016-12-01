/* ************************* scanner.c **************************************/
/* Projekt:             Implementace interpretu jazyka IFJ16				*/
/* Predmet:             Formalni jazyky a prekladace (IFJ)					*/
/* Soubor:              test_scanner.c - Lexikalni analyzator				*/
/* Kodovani:            UTF-8												*/
/* Datum:               													*/
/* Varianta zadani:															*/
/* Autori, login:       				      								*/
/*                     														*/
/*																			*/
/*																			*/
/*																			*/
/* **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

#include "str.h"
#include "test_scanner.h"
#include "test_error.h"
#include "buffer.h"
// pocet klicovych slov
#define NUM_KEYW 11
// klicova slova
char * key_words[] = {	"break", "class", "continue", "do", "else", "false", "for",
						"if", "return", "static", "true", "while"};
// pocet radku kodu
int line = 1;

/*	Stavy konecneho avtomatu lexikalniho analyzatoru	*/
typedef enum{

    R_NEW,			// cteni noveho symboly						(0)
    R_ID_OR_KW,		// cteni identifikatoru nebo klicoveho slova(1)
    R_NUMBER,		// cteni cisla								(2)
    R_DBNUM,		// cteni desetinneho cisla					(3)
    R_DBNUM2,		// cteni desetinneho cisla					(4)
    R_STRING,		// cteni retezce 							(6)
    R_SYM,			// cteni symbolu							(7)
    DIV_OR_COMM,	// cteni deleni nebo komentaru				(8)
    OP_OR_INC,		// cteni operatoru nebo incrementu			(9)
    OP_OR_DEC,		// cteni operatoru nebo decrementu			(10)
    OP_OR_ASS,		// cteni prirazeni nebo porovnani			(11)
    LESS_OR_LOEQ,	// cteni znaku mensi nebo mensi-ravno		(12)
    GR_OR_GOEQ,		// cteni znaku vetsi nebo vetsi-ravno		(13)
    NOT_OR_NEQ,		// cteni negace nebo nonekvivalence			(14)
	R_OR,
	R_AND,
    STR_COMMENT,	// retezcovy komentar 						(15)
    BL_COMMENT,		// blokovy komentar 						(16)
    BL_COMMENT_2,	// konec blokoveho komentare				(17)

}sState;

// zdrojovy soubor
FILE *file;
// stav zapisu retezce
bool isWr = false;
// stav zapisu doubl s E/e
bool isD = false;

/*	Funkce zapisu symbolu do atributu tokenu	*/
void editAtt(string *s1, char c)
{
	if ((strAddChar(s1, c)))
	{
		line++;
		throw_err(INT_ERROR, ALL_STRUCT, 0);
	}
}

/*	Nastavi soubor se ktereho scanner bude cist lexemy		*/
void open_source(const char *file_name)
{
	if ((file = fopen(file_name, "r")) == NULL)
	{
		throw_err(INT_ERROR, 0, 0);
	}
}

void close_source()
{
	fclose(file);
}

/*	Funce zpracovani lexemu		*/
void get_token(Token *token)
{

	// vymaze obsah attributu tokenu
	if (token != NULL)
	{
		strClear(&token->attr);
		strFree(&token->attr);
	}
	// stav automatu
	sState state = R_NEW;

	strInit(&token->attr);

	while (1)
	{
		// precteny symbol
		char c = getc(file);
		// kontrola stavu
		switch (state)
		{
			// novy token
			case R_NEW:
				// bile znaky
				if (isspace(c))
				{
					if (c == 10)			// kontrola noveho radku
						line++;
				}
				// klicove slovo nebo identifikator
				else if (isalpha(c) || c == '_' || c == '$')
				{
					editAtt(&token->attr, c);
					state = R_ID_OR_KW;
				}
				// cele nebo desetinne cislo
				else if (c >= 48 && c <= 57)
				{
					editAtt(&token->attr, c);
					state = R_NUMBER;
				}
				else if (c == '.')
				{
					editAtt(&token->attr, c);
					token->type = POINT;
					return;
				}
				else if (c == ',')
				{
					editAtt(&token->attr, c);
					token->type = COMMA;
					return;
				}
				else if (c == ';')
				{
					editAtt(&token->attr, c);
					token->type = SEMICOLON;
					return;
				}

				else if (c == '(')
				{
					editAtt(&token->attr, c);
					token->type = L_PAR;
					return;
				}

				else if (c == ')')
				{
					editAtt(&token->attr, c);
					token->type = R_PAR;
					return;
				}

				else if (c == '{')
				{
					editAtt(&token->attr, c);
					token->type = L_VIN;
					return;
				}

				else if (c == '}')
				{
					editAtt(&token->attr, c);
					token->type = R_VIN;
					return;
				}

				else if (c == '*')
				{
					editAtt(&token->attr, c);
					token->type = MUL;
					return;
				}

				else if (c == '"')
				{
					if (isWr) isWr = false;
					editAtt(&token->attr, c);
					state = R_STRING;				// cteni retezce
				}

				else if (c == '\'')
				{
					editAtt(&token->attr, c);
					state = R_SYM;		// cteni symbolu
				}

				else if (c == '/')
					state = DIV_OR_COMM;	// deleni nebo komentar

				else if (c == '+')
				{						// plus nebo incrementace
					editAtt(&token->attr, c);
					state = OP_OR_INC;
				}

				else if (c == '-')
				{						// minus nebo decrementace
					editAtt(&token->attr, c);
					state = OP_OR_DEC;
				}

				else if (c == '=')
				{						// prirazeni nebo porovnani
					editAtt(&token->attr, c);
					state = OP_OR_ASS;
				}

				else if (c == '<')
				{						// mensi nebo mensi-ravno
					editAtt(&token->attr, c);
					state = LESS_OR_LOEQ;
				}

				else if (c == '>')
				{						// vetsi nebo vetsi-ravno
					editAtt(&token->attr, c);
					state = GR_OR_GOEQ;
				}

				else if (c == '!')
				{						// negace nebo nonekvivalence
					editAtt(&token->attr, c);
					state = NOT_OR_NEQ;
				}

				else if (c == '|')
				{
					editAtt(&token->attr, c);
					state = R_OR;
				}

				else if (c == '&')
				{
					editAtt(&token->attr, c);
					state = R_AND;
				}

				else if (c == EOF)
				{
					token->type = T_EOF;
					return;
				}
			break;

			// klicove slovo nebo identifikator
			case R_ID_OR_KW:

				if (isalnum(c) || c == '_' || c == '$' || c == '.')
					editAtt(&token->attr, c);

				else
				{
					// vraceni symbolu zpet
					ungetc(c, file);
					for (int i = 0; i <= NUM_KEYW; i++)
					{
						if (!strCmpConstStr(&token->attr, key_words[i]))
						{
							token->type = KEYWORD;
							return;
						}
	    			}
	    			if (!strCmpConstStr(&token->attr, "int"))
					{
						token->type = INT;
						return;
					}
					else if (!strCmpConstStr(&token->attr, "double"))
					{
						token->type = DOUBLE;
						return;
					}
					else if (!strCmpConstStr(&token->attr, "String"))
					{
						token->type = STRING;
						return;
					}
					else if (!strCmpConstStr(&token->attr, "void"))
					{
						token->type = VOID;
						return;
					}
					else if (!strCmpConstStr(&token->attr, "boolean"))
					{
						token->type = BOOLEAN;
						return;
					}
	    			// jinak to bude identifikator
	    			token->type = IDENTIFIER;
					return;
				}
			break;

			// cislo (desetinne nebo cele)
			case R_NUMBER:

				if (c == 'e' || c == 'E')
				{
					editAtt(&token->attr, c);
					state = R_DBNUM2;
				}
				// pokud cislo obsahuje desetinnou tecku, nastavi cteni desetineho cisla
				else if (c == '.')
				{
					editAtt(&token->attr, c);
					state = R_DBNUM;
				}
				else if (c >= 48 && c <= 57)
				{
					editAtt(&token->attr, c);
				}
				else if (c >= 63)
				{
					line++;
					editAtt(&token->attr, c);
					throw_err(LEX_ERROR, UNK_LEX, token->attr.str);
				}
				else
				{
					ungetc(c, file);
					token->type = VALUE;
					return;
				}
			break;

			// desetinne cislo (1)
			case R_DBNUM:

				if (c >= 48 && c <= 57)
				{
					editAtt(&token->attr, c);
				}
				else if (c == 'e' || c == 'E')
				{
					editAtt(&token->attr, c);
					state = R_DBNUM2;
				}
				else if (c >= 63)
					return;
				else
				{
					ungetc(c, file);
					token->type = VALUE;
					return;
				}
			break;

			// desetinne cislo (1)
			case R_DBNUM2:

				if ((c == '+' || c == '-') && !isD)
				{
					editAtt(&token->attr, c);
					isD = true;
				}
				else if (c >= 48 && c <= 57)
				{
					editAtt(&token->attr, c);
					isD = false;
				}
				else if ((c >= 48 && c <= 57 && !isD) || c >= 63)
				{
					line++;
					editAtt(&token->attr, c);
					throw_err(LEX_ERROR, UNK_LEX, token->attr.str);
				}
				else
				{
					if (isD)
					{
						editAtt(&token->attr, c);
						throw_err(LEX_ERROR, UNK_LEX, token->attr.str);
					}
					ungetc(c, file);
					token->type = VALUE;
					return;
				}
			break;

			// retezec
			case R_STRING:
				if (c == EOF)
				{
					line++;
					editAtt(&token->attr, c);
					throw_err(LEX_ERROR, UNK_LEX, token->attr.str);
				}
				else if (c == '\\')
				{
					editAtt(&token->attr, c);
					isWr = true;
				}
				else if ((c == '"' && isWr) || c != '"')
				{
					if (isWr) isWr = false;
					editAtt(&token->attr, c);
				}
				else
				{
					editAtt(&token->attr, c);
					token->type = VALUE;
					return;
				}
			break;

			// symbol
			case R_SYM:
				if (c != '\'' && strGetLength(&token->attr) < 2)
				{
					editAtt(&token->attr, c);
				}
				else if (c == '\'' && strGetLength(&token->attr) == 2)
				{
					editAtt(&token->attr, c);
					token->type = VALUE;
					return;
				}
				else
				{
					line++;
					editAtt(&token->attr, c);
					throw_err(LEX_ERROR, UNK_LEX, token->attr.str);
				}
			break;

			// deleni nebo komentar
			case DIV_OR_COMM:

				if (c == '/') state = STR_COMMENT;
				else if (c == '*') state = BL_COMMENT;
				else
				{
					ungetc(c, file);
					editAtt(&token->attr, '/');
					token->type = DIV;
					return;
				}
			break;

			// plus nebo increment
			case OP_OR_INC:
				if (c == '+')
				{
					editAtt(&token->attr, c);
					token->type = INCREMENT;
				}
				else
				{
					ungetc(c, file);
					token->type = PLUS;
				}
				return;

			// minus nebo decrement
			case OP_OR_DEC:
				if (c == '-')
				{
					editAtt(&token->attr, c);
					token->type = DECREMENT;
				}
				else
				{
					ungetc(c, file);
					token->type = MINUS;
				}
				return;

			// prirazeni nebo porovnani
			case OP_OR_ASS:
				if (c == '=')
				{
					editAtt(&token->attr, c);
					token->type = COMPARISON;
				}
				else
				{
					ungetc(c, file);
					token->type = ASSIGNMENT;
				}
				return;

			// mensi nebo mensi-rovno
			case LESS_OR_LOEQ:
				if (c == '=')
				{
					editAtt(&token->attr, c);
					token->type = LOEQ;
				}
				else
				{
					ungetc(c, file);
					token->type = LESS;
				}
				return;

			// vetsi nebo vetsi-rovno
			case GR_OR_GOEQ:
				if (c == '=')
				{
					editAtt(&token->attr, c);
					token->type = GOEQ;
				}
				else
				{
					ungetc(c, file);
					token->type = GREATER;
				}
				return;

			// negace nebo nonekvivalence
			case NOT_OR_NEQ:
				if (c == '=')
				{
					editAtt(&token->attr, c);
					token->type = NEQ;
				}
				else
				{
					ungetc(c, file);
					token->type = NOT;
				}
				return;

			// operace nebo
			case R_OR:
				if (c == '|')
				{
					editAtt(&token->attr, c);
					token->type = OR;
				}
				else
				{
					line++;
					editAtt(&token->attr, c);
					throw_err(LEX_ERROR, UNK_LEX, token->attr.str);
				}
				return;

			// operace nebo
			case R_AND:
				if (c == '&')
				{
					editAtt(&token->attr, c);
					token->type = AND;
				}
				else
				{
					line++;
					editAtt(&token->attr, c);
					throw_err(LEX_ERROR, UNK_LEX, token->attr.str);
				}
				return;

			// retezcovu komentar
			case STR_COMMENT:
				if ((int) c == 10)
					state = R_NEW;
			break;

			// blokovy komentar (1)
			case BL_COMMENT:
				if (c == '*')
					state = BL_COMMENT_2;
			break;

			// blokovy komentar (2)
			case BL_COMMENT_2:
				if (c == '/')
					state = R_NEW;
				else
					state = BL_COMMENT;
			break;
		}
	}
}

void go_back(Token *token)
{
	fseek(file, token->attr.length * (-1) , SEEK_CUR);
}

/*	Zniceni tokenu	*/
void free_token(Token *token)
{
	strFree(&token->attr);
	free_pointer(token, true);
}
