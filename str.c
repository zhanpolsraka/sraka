#include <stdbool.h>
#include "str.h"
#include "error.h"
#include "buffer.h"

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
      throw_err(INT_ERROR, ALL_STRUCT, 0);
      mark_mem(s->str);
   s->str[0] = '\0';
   s->length = 0;
   s->allocSize = STR_LEN_INC;
   return STR_SUCCESS;
}

void strFree(string *s)
// funkce uvolni retezec z pameti
{
   free_pointer(s->str, true);
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
      free_pointer(s1->str, false);
      if ((s1->str = (char*) realloc(s1->str, s1->length + STR_LEN_INC)) == NULL)
         throw_err(INT_ERROR, ALL_STRUCT, 0);
         mark_mem(s1->str);
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
      free_pointer(s1->str, false);
      if ((s1->str = (char*) realloc(s1->str, newLength + 1)) == NULL)
         throw_err(INT_ERROR, ALL_STRUCT, 0);
         mark_mem(s1->str);
      s1->allocSize = newLength + 1;
   }
    strcpy(s1->str, s2->str);
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
