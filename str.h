#include <string.h>
#include <malloc.h>
//hlavickovy soubor pro praci s nekonecne dlouhymi retezci

typedef struct{

  char* str;		// misto pro dany retezec ukonceny znakem '\0'
  int length;		// skutecna delka retezce
  int allocSize;	// velikost alokovane pameti

}string;


int strInit(string *s);
void strFree(string *s);

void strClear(string *s);
int strAddChar(string *s1, char c);
int strWriteStr(string *s1, char *str);
int strCopyString(string *s1, string *s2);
int strCmpString(string *s1, string *s2);
int strCmpConstStr(string *s1, char *s2);

int strAddStr(string *s1, string *s2);
bool equal_str(char *s1, char *s2);
char *strGetStr(string *s);
int strGetLength(string *s);
