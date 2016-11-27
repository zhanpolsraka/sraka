#ifndef _GARBAGE_H_
#define _GARBAGE_H_

void mark_tokens(Token *main_token, tHelpVar *h_token);
void mark_binary_tree(tTree *tree);
void mark_instr_stack(tInstrStack *st);
void mark_expr_stack(tExprStack *st);
void mark_frame_stack(tFrameStack *st);
void free_all();

#endif
