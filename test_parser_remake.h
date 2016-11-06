#ifndef _PARSER_H_
#define _PARSER_H_

extern bool condition;

int parsing_succesful();
bool class_list(Token *token);
bool class(Token *token);
bool class_body(Token *token);
bool function(Token *token);
bool argument_list(Token *token);
bool next_arg(Token *token);
bool statement_list(Token *token);
bool ride_struct(Token *token);
bool define_var(Token *token);
bool init_var(Token *token);


#endif
