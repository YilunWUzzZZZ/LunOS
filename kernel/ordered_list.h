#ifndef ORDERED_LIST_H
#define ORDERED_LIST_H

#include "types.h"

typedef void * type_t;
typedef sint8 (*comparator_t)(type_t, type_t);
//return nonzero if the first is less than the second

typedef struct
{
   type_t *array;
   uint32 size;
   uint32 max_size;
   comparator_t less_than;
} ordered_array_t;

sint8 standard_comparator(type_t a, type_t b);

ordered_array_t create_ordered_array(uint32 max_size, comparator_t less_than);
ordered_array_t place_ordered_array(void * addr, uint32 max_size, comparator_t less_than);

void insert_ordered_array(type_t item, ordered_array_t *array);
type_t lookup_ordered_array(uint32 i, ordered_array_t *array);
void remove_ordered_array(uint32 i, ordered_array_t *array);
uint32 search_ordered_array(type_t item, ordered_array_t *array);

#endif