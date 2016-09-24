#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "test_scanner.h"

Token get_token(FILE *file){
	
	int state = 0;			// stav automatu

	Token token;
	if((token.name = (char *) malloc(10)) == NULL){
		printf("%s\n", "Error name");
		exit(0);
	}
	if((token.type = (char *) malloc(5)) == NULL){
		printf("%s\n", "Error type");
		exit(0);
	}
	token.len_name = 0;
	token.type = "lex";

	while(1){

		char c = getc(file);	// precteny symbol

		// kontrola stavu
		switch(state){

			printf("%s\n", "start switch");

			// novy token
			case 0:
				printf("%s %c%c%c\n", "start state 0", '\'', c, '\'');

				if(isspace(c)){}				// bile znaky
				else if(isalnum(c)){			// klicove slovo nebo identifikator
					state = 1;
					token.name[token.len_name] = c;
					token.len_name++;
				}
				else if(c == '/') state = 2;	// komentar nebo deleni
				else if(c == '{'){
					token.name = "l_vin";
					return token;
				}
				else if(c == '}'){
					token.name = "r_vin";
					return token;
				}
				else if(c == '+'){
					token.name = "plus";
					return token;
				}
				else if(c == '-'){
					token.name = "minus";
					return token;
				}
				else if(c == '*'){
					token.name = "multipl";
					return token;
				}
				else if(c == '='){
					token.name = "equal";
					return token;
				}
				else{
					token.name = "vse";
					return token;
				}
				break;

			// klicove slovo nebo identifikator	
			case 1:
				printf("%s %c%c%c\n", "start state 1", '\'', c, '\'');

				if(isalnum(c)){
					token.name[token.len_name] = c;
					token.len_name++;
				}
				else{
					
					if(!strcmp(token.name, "int")){
						token.type = "int";
					}
					else if(!strcmp(token.name, "double")){
						token.type = "doubl";
					}
					state = 0;
					return token;
				}
				break;

			// komentare
			case 2:
				printf("%s %c%c%c\n", "start state 2", '\'', c, '\'');
				if(c == '/') state = 0;
		}
	}
}


int main(int argc, char *argv[]){

	FILE *file;

	if((file = fopen(argv[1], "r")) == NULL) printf("Error\n");

	Token array[50];
	int i = 0;
	while(!feof(file)){
		array[i] = get_token(file);
		printf("%s\n", "got!");

		printf("%s %s\n", array[i].name, array[i].type);
		
		i++;
	}
	
	return 0;
}