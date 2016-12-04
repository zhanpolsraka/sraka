/* **************************************************************************/
/* Projekt:             Implementace interpretu jazyka IFJ16				*/
/* Predmet:             Formalni jazyky a prekladace (IFJ)					*/
/* Soubor:              buffer.c  (Prace s ukazateli na alokovanou pamet)	*/
/*																			*/
/* Autor login:      	Ermak Aleksei		xermak00						*/
/*                     	Khaitovich Anna		xkhait00						*/
/*						Nesmelova Antonina	xnesmel00						*/
/*						Fedorenko Oleh		xfedor07						*/
/*						Fedin Evgenii		xfedin00						*/
/* **************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "error.h"
#include "buffer.h"

tBuffer *buff = NULL;

void create_buffer()
{
    if ((buff = malloc(sizeof(tBuffer))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
    buff->size = 20;
    buff->number = -1;
    if ((buff->array = malloc(buff->size * sizeof(void *))) == NULL)
        throw_err(INT_ERROR, ALL_STRUCT, 0);
}

void mark_mem(void *place)
{
    if (buff->number < buff->size - 1)
    {
        buff->number++;
        buff->array[buff->number] = place;
    }
    else
    {
        buff->size += 20;
        if ((buff->array = realloc(buff->array, buff->size * sizeof(void *))) == NULL)
            throw_err(INT_ERROR, ALL_STRUCT, 0);
        mark_mem(place);
    }
}

void clear_buffer()
{
    while (buff->number > -1)
    {
        //printf("number [%d] -> [%d]\n", buff->number, *buff->array[buff->number] != NULL);
        if (buff->array[buff->number])
        {
            free(buff->array[buff->number]);
        }
        buff->number--;
    }
    free(buff->array);
    free(buff);
}

void free_pointer(void *pointer, bool full)
{
    // vyhledavame ukazatel
    for (int i = buff->number; i > -1; i--)
    {
        if (buff->array[i] == pointer)
        {
            buff->array[i] = 0;
            break;
        }
    }
    if (full)
        free(pointer);
}
