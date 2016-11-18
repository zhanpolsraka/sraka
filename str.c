#include <stdbool.h>
#include <stdarg.h>
#include "str.h"
#include "test_error.h"
//jednoducha knihovna pro praci s nekonecne dlouhymi retezci

#define STR_LEN_INC 8
// konstanta STR_LEN_INC udava, na kolik bytu provedeme pocatecni alokaci pameti
// pokud nacitame retezec znak po znaku, pamet se postupne bude alkokovat na
// nasobky tohoto cisla

#define STR_ERROR   1
#define STR_SUCCESS 0

int strInit(string *s)
// funkce vytvori novy retezec
{
   if ((s->str = (char*) malloc(STR_LEN_INC)) == NULL)
      return STR_ERROR;
   s->str[0] = '\0';
   s->length = 0;
   s->allocSize = STR_LEN_INC;
   return STR_SUCCESS;
}

void strFree(string *s)
// funkce uvolni retezec z pameti
{
   free(s->str);
}

void strClear(string *s)
// funkce vymaze obsah retezce
{
   s->str[0] = '\0';
   s->length = 0;
}

int strAddChar(string *s1, char c)
// prida na konec retezce jeden znak
{
   if (s1->length + 1 >= s1->allocSize)
   {
      // pamet nestaci, je potreba provest realokaci
      if ((s1->str = (char*) realloc(s1->str, s1->length + STR_LEN_INC)) == NULL)
         return STR_ERROR;
      s1->allocSize = s1->length + STR_LEN_INC;
   }
   s1->str[s1->length] = c;
   s1->length++;
   s1->str[s1->length] = '\0';
   return STR_SUCCESS;
}

int strWriteStr(string *s1, char *str)
{
    while (*str)
    {
        strAddChar(s1, *str);
        str++;
    }
    return STR_SUCCESS;
}

bool equal_str(char *s1, char *s2)
{
     return !strcmp(s1, s2);
}

int strCopyString(string *s1, string *s2)
// prekopiruje retezec s2 do s1
{
   int newLength = s2->length;
   if (newLength >= s1->allocSize)
   {
      // pamet nestaci, je potreba provest realokaci
      if ((s1->str = (char*) realloc(s1->str, newLength + 1)) == NULL)
         return STR_ERROR;
      s1->allocSize = newLength + 1;
   }
   strcpy(s1->str, s2->str);
   s1->length = newLength;
   return STR_SUCCESS;
}

int strCopyConstString(string *s1, char *s2)
// prekopiruje retezec s2 do s1
{
   int newLength = strlen(s2);
   if (newLength >= s1->allocSize)
   {
      // pamet nestaci, je potreba provest realokaci
      if ((s1->str = (char*) realloc(s1->str, newLength + 1)) == NULL)
         return STR_ERROR;
      s1->allocSize = newLength + 1;
   }
   strcpy(s1->str, s2);
   s1->length = newLength;
   return STR_SUCCESS;
}

int strCmpString(string *s1, string *s2)
// porovna oba retezce a vrati vysledek
{
   return strcmp(s1->str, s2->str);
}

int strCmpConstStr(string *s1, char* s2)
// porovna nas retezec s konstantnim retezcem
{
   return strcmp(s1->str, s2);
}

char *strGetStr(string *s)
// vrati textovou cast retezce
{
   return s->str;
}

int strGetLength(string *s)
// vrati delku daneho retezce
{
   return s->length;
}

/**
* TODO
* Function returns:
*   NULL if something went wrong.
*   (string *) if everything is ok.
**/
string *new_string(char *init) {
    string *new_str = malloc(sizeof(string));
    if (new_str != NULL) {
        if (strInit(new_str) == STR_ERROR) {
            return NULL;
        };
    }
    if (strCopyConstString(new_str, init) == STR_ERROR) {
        return NULL;
    }
    return new_str;
}

/**
* TODO
*
**/
void del_strings(unsigned int count, ...) {
    string *elem_to_delete;
    va_list arg_ptr;

    va_start(arg_ptr, count);
    while (count--) {
        elem_to_delete = va_arg(arg_ptr, string *);
        strFree(elem_to_delete);
        free(elem_to_delete);
    }
    va_end(arg_ptr);
}
