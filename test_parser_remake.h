#ifndef _PARSER_H_
#define _PARSER_H_

extern int condition;

void parsing();
void class_list(Token *token);
void class(Token *token);
void class_body(Token *token);
void function(Token *token);
void argument_list(Token *token);
void next_arg(Token *token);
void call_arg_list(Token *token);
void call_next_arg(Token *token);
void statement_list(Token *token);
void ride_struct(Token *token);
void define_var(Token *token);
void init_var(Token *token);


#endif
