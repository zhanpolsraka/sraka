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
// pocet klicovych slov
#define NUM_KEYW 12
// klicova slova
char * key_words[] = {	"break", "class", "continue", "do", "else", "false", "for",
						"if", "return", "static", "true", "while", "print"};
// pocet radku kodu
int line = 1;

/*	Stavy konecneho avtomatu lexikalniho analyzatoru	*/
typedef enum{

    R_NEW,			// cteni noveho symboly						(0)
    R_ID_OR_KW,		// cteni identifikatoru nebo klicoveho slova(1)
    R_NUMBER,		// cteni cisla								(2)
    R_DBNUM,		// cteni desetinneho cisla					(3)
    R_DBNUM2,		// cteni desetinneho cisla					(4)
    R_ESCSEQ,		// cteni escape sekvence					(5)
    R_STRING,		// cteni retezce 							(6)
    R_SYM,			// cteni symbolu							(7)
    DIV_OR_COMM,	// cteni deleni nebo komentaru				(8)
    OP_OR_INC,		// cteni operatoru nebo incrementu			(9)
    OP_OR_DEC,		// cteni operatoru nebo decrementu			(10)
    OP_OR_ASS,		// cteni prirazeni nebo porovnani			(11)
    LESS_OR_LOEQ,	// cteni znaku mensi nebo mensi-ravno		(12)
    GR_OR_GOEQ,		// cteni znaku vetsi nebo vetsi-ravno		(13)
    NOT_OR_NEQ,		// cteni negace nebo nonekvivalence			(14)
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
int editAtt(string *s1, char c){
	if((strAddChar(s1, c))){
		line++;
		throw_err(ALLOC_ERROR, ALL_STRUCT);
		return 1;
	}
	return 0;
}

/*	Nastavi soubor se ktereho scanner bude cist lexemy		*/
void open_source(const char *file_name){
	if((file = fopen(file_name, "r")) == NULL){
		throw_err(OPEN_FILE, FILE_ERROR);
	}
}

void close_source(){
	fclose(file);
}

/*	Funce zpracovani lexemu		*/
bool get_token(Token *token){

	// vymaze obsah attributu tokenu
	if (token != NULL)
	{
		strClear(&token->attr);
		strFree(&token->attr);
	}
	// stav automatu
	sState state = R_NEW;

	if(strInit(&token->attr)){
		throw_err(ALLOC_ERROR, ALL_STRUCT);
		return false;
	}
	while(1){
		// precteny symbol
		char c = getc(file);
		// kontrola stavu
		switch(state){
			// novy token
			case R_NEW:
				// bile znaky
				if(isspace(c)){
					if(c == 10)			// kontrola noveho radku
						line++;
				}
				// pokud jde zapis retezcu a c je zpetny slash, zacne zapis escape sekvence
				else if(isWr && c == '\\'){
					if(editAtt(&token->attr, c)) return false;
					state = R_ESCSEQ;
				}
				// klicove slovo nebo identifikator
				else if(isalpha(c) || c == '_' || c == '$'){
					if(editAtt(&token->attr, c)) return false;
					state = R_ID_OR_KW;
				}
				// cele nebo desetinne cislo
				else if(c >= 48 && c <= 57){
					if(editAtt(&token->attr, c)) return false;
					state = R_NUMBER;
				}
				else if(c == '.'){
					if(editAtt(&token->attr, c)) return false;
					token->type = POINT;
					return true;
				}
				else if(c == ','){
					if(editAtt(&token->attr, c)) return false;
					token->type = COMMA;
					return true;
				}
				else if(c == ';'){
					if(editAtt(&token->attr, c)) return false;
					token->type = SEMICOLON;
					return true;
				}

				else if(c == '('){
					if(editAtt(&token->attr, c)) return false;
					token->type = L_PAR;
					return true;
				}

				else if(c == ')'){
					if(editAtt(&token->attr, c)) return false;
					token->type = R_PAR;
					return true;
				}

				else if(c == '{'){
					if(editAtt(&token->attr, c)) return false;
					token->type = L_VIN;
					return true;
				}

				else if(c == '}'){
					if(editAtt(&token->attr, c)) return false;
					token->type = R_VIN;
					return true;
				}

				else if(c == '*'){
					if(editAtt(&token->attr, c)) return false;
					token->type = MUL;
					return true;
				}

				else if(c == '"'){
					if(isWr) isWr = false;
					else state = R_STRING;				// cteni retezce
				}

				else if(c == '\'') state = R_SYM;		// cteni symbolu
				else if(c == '/') state = DIV_OR_COMM;	// deleni nebo komentar
				else if(c == '+'){						// plus nebo incrementace
					if(editAtt(&token->attr, c)) return false;
					state = OP_OR_INC;
				}
				else if(c == '-'){						// minus nebo decrementace
					if(editAtt(&token->attr, c)) return false;
					state = OP_OR_DEC;
				}
				else if(c == '='){						// prirazeni nebo porovnani
					if(editAtt(&token->attr, c)) return false;
					state = OP_OR_ASS;
				}
				else if(c == '<'){						// mensi nebo mensi-ravno
					if(editAtt(&token->attr, c)) return false;
					state = LESS_OR_LOEQ;
				}
				else if(c == '>'){						// vetsi nebo vetsi-ravno
					if(editAtt(&token->attr, c)) return false;
					state = GR_OR_GOEQ;
				}
				else if(c == '!'){						// negace nebo nonekvivalence
					if(editAtt(&token->attr, c)) return false;
					state = NOT_OR_NEQ;
				}
				else if(c == EOF){
					token->type = T_EOF;
					return true;
				}
				break;

			// klicove slovo nebo identifikator
			case R_ID_OR_KW:

				if(isalnum(c) || c == '_' || c == '$' || c == '.'){
					if(editAtt(&token->attr, c)) return false;
				}
				else{
					// vraceni symbolu zpet
					ungetc(c, file);
					for(int i = 0; i <= NUM_KEYW; i++){
						if (!strCmpConstStr(&token->attr, key_words[i])){
							token->type = KEYWORD;
							return true;
						}
	    			}
	    			if(!strCmpConstStr(&token->attr, "int")){
						token->type = INT;
						return true;
					}
					else if(!strCmpConstStr(&token->attr, "double")){
						token->type = DOUBLE;
						return true;
					}
					else if(!strCmpConstStr(&token->attr, "String")){
						token->type = STRING;
						return true;
					}
					else if(!strCmpConstStr(&token->attr, "void")){
						token->type = VOID;
						return true;
					}
					else if(!strCmpConstStr(&token->attr, "bool")){
						token->type = BOOLEAN;
						return true;
					}
	    			// jinak to bude identifikator
	    			token->type = IDENTIFIER;
					return true;
				}
				break;

			// cislo (desetinne nebo cele)
			case R_NUMBER:

				if(c == 'e' || c == 'E'){
					if(editAtt(&token->attr, c)) return false;
					state = R_DBNUM2;
				}
				// pokud cislo obsahuje desetinnou tecku, nastavi cteni desetineho cisla
				else if(c == '.'){
					if(editAtt(&token->attr, c)) return false;
					state = R_DBNUM;
				}
				else if(c >= 48 && c <= 57){
					if(editAtt(&token->attr, c)) return false;
				}
				else if(c >= 63){
					line++;
					throw_err(LEX_ERROR, UNK_LEX);
					return false;
				}
				else{
					ungetc(c, file);
					token->type = VALUE;
					return true;
				}
				break;

			// desetinne cislo (1)
			case R_DBNUM:

				if(c >= 48 && c <= 57){
					if(editAtt(&token->attr, c)) return false;
				}
				else if(c == 'e' || c == 'E'){
					if(editAtt(&token->attr, c)) return false;
					state = R_DBNUM2;
				}
				else if(c >= 63) return false;
				else{
					ungetc(c, file);
					token->type = VALUE;
					return true;
				}
				break;

			// desetinne cislo (1)
			case R_DBNUM2:

				if((c == '+' || c == '-') && !isD){
					if(editAtt(&token->attr, c)) return false;
					isD = true;
				}
				else if(c >= 48 && c <= 57 && isD){
					if(editAtt(&token->attr, c)) return false;
				}
				else if((c >= 48 && c <= 57 && !isD) || c >= 63){
					line++;
					throw_err(LEX_ERROR, UNK_LEX);
					return false;
				}
				else{
					ungetc(c, file);
					token->type = VALUE;
					isD = false;
					return true;
				}
				break;

			// escape sekvence
			case R_ESCSEQ:
				if(c == 'n' || c == 't' || c == '\\' || c == '"'){
					if(editAtt(&token->attr, c)) return false;
					token->type = ESCSEQ;
					return true;
				}
				else if(c >= 48 && c <= 57 && strGetLength(&token->attr) < 4){
					if(editAtt(&token->attr, c)) return false;
				}
				else if(strGetLength(&token->attr) == 4){
					ungetc(c, file);
					token->type = ESCSEQ;
					return true;
				}
				else{
					line++;
					throw_err(LEX_ERROR, UNK_LEX);
					return false;
				}
				break;

			// retezec
			case R_STRING:

				if(c != '"'){
					if(c == EOF){
						line++;
						throw_err(LEX_ERROR, UNK_LEX);
						return false;
					}
					else if(c == '\\'){
						ungetc(c, file);
						token->type = VALUE;
						isWr = true;						// napoveda programmu, ze jde zapis retezce
						return true;
					}
					else if(editAtt(&token->attr, c)) return false;
				}
				else{
					isWr = false;
					token->type = VALUE;
					return true;
				}
				break;

			// symbol
			case R_SYM:
				if(c != '\'' && strGetLength(&token->attr) < 1){
					if(editAtt(&token->attr, c)) return false;
				}
				else if(c == '\'' && strGetLength(&token->attr) == 1){
					token->type = VALUE;
					return true;
				}
				else{
					line++;
					throw_err(LEX_ERROR, UNK_LEX);
					return false;
				}
				break;

			// deleni nebo komentar
			case DIV_OR_COMM:

				if(c == '/') state = STR_COMMENT;
				else if(c == '*') state = BL_COMMENT;
				else{
					ungetc(c, file);
					if(editAtt(&token->attr, '/')) return false;
					token->type = DIV;
					return true;
				}
				break;

			// plus nebo increment
			case OP_OR_INC:
				if(c == '+'){
					if(editAtt(&token->attr, c)) return false;
					token->type = INCREMENT;
				}
				else{
					ungetc(c, file);
					token->type = PLUS;
				}
				return true;

			// minus nebo decrement
			case OP_OR_DEC:
				if(c == '-'){
					if(editAtt(&token->attr, c)) return false;
					token->type = DECREMENT;
				}
				else{
					ungetc(c, file);
					token->type = MINUS;
				}
				return true;

			// prirazeni nebo porovnani
			case OP_OR_ASS:
				if(c == '='){
					if(editAtt(&token->attr, c)) return false;
					token->type = COMPARISON;
				}
				else{
					ungetc(c, file);
					token->type = ASSIGNMENT;
				}
				return true;

			// mensi nebo mensi-rovno
			case LESS_OR_LOEQ:
				if(c == '='){
					if(editAtt(&token->attr, c)) return false;
					token->type = LOEQ;
				}
				else{
					ungetc(c, file);
					token->type = LESS;
				}
				return true;

			// vetsi nebo vetsi-rovno
			case GR_OR_GOEQ:
				if(c == '='){
					if(editAtt(&token->attr, c)) return false;
					token->type = GOEQ;
				}
				else{
					ungetc(c, file);
					token->type = GREATER;
				}
				return true;

			// negace nebo nonekvivalence
			case NOT_OR_NEQ:
				if(c == '='){
					if(editAtt(&token->attr, c)) return false;
					token->type = NEQ;
				}
				else{
					ungetc(c, file);
					token->type = NOT;
				}
				return true;

			// retezcovu komentar
			case STR_COMMENT:
				if((int) c == 10) state = R_NEW;
				break;

			// blokovy komentar (1)
			case BL_COMMENT:
				if(c == '*') state = BL_COMMENT_2;
				break;

			// blokovy komentar (2)
			case BL_COMMENT_2:
				if(c == '/') state = R_NEW;
				else state = BL_COMMENT;
				break;
		}
	}
}

void go_back(Token *token)
{
	fseek(file, token->attr.length * (-1) , SEEK_CUR);
}

bool init_token(Token *token)
{
	if ((token = malloc(sizeof(Token))) == NULL)
	{
		throw_err(ALLOC_ERROR, ALL_STRUCT);
		return false;
	}
	strInit(&token->attr);
	return true;
}
/*	Zniceni tokenu	*/
void free_token(Token *token){
	strFree(&token->attr);
	free(token);
}
