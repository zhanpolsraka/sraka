#ifndef _BUFFER_H_
#define _BUFFER_H_

typedef struct
{
    void **array;
    int size;
    int number;

}tBuffer;

void mark_mem(void *place);
void create_buffer();
void clear_buffer();
void free_pointer(void *pointer, bool full);

#endif
