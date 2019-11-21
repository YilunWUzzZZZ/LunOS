#include "ordered_list.h"
#include "kheap.h"
#include "utils.h"
#include "common.h"

sint8 standard_comparator(type_t a, type_t b)
{
    return (a<b) ? 1 : 0;
}

ordered_array_t create_ordered_array(uint32 max_size, comparator_t less_than)
{
   ordered_array_t to_ret;
   to_ret.array = (void*)kmalloc(max_size*sizeof(type_t), 0);
   memset(to_ret.array, 0, max_size*sizeof(type_t));
   to_ret.size = 0;
   to_ret.max_size = max_size;
   to_ret.less_than = less_than;
   return to_ret;
}

ordered_array_t place_ordered_array(void *addr, uint32 max_size, comparator_t less_than)
{
   ordered_array_t to_ret;
   to_ret.array = (type_t*)addr;
   memset(to_ret.array, 0, max_size*sizeof(type_t));
   to_ret.size = 0;
   to_ret.max_size = max_size;
   to_ret.less_than = less_than;
   return to_ret;
}

void insert_ordered_array(type_t item, ordered_array_t *array)
{
   ASSERT(array->size < array->max_size);
   uint32 iterator = 0;
   while (iterator < array->size && array->less_than(array->array[iterator], item))
       iterator++;
   if (iterator == array->size) // just add at the end of the array.
       array->array[array->size++] = item;
   else
   {
       type_t tmp = array->array[iterator];
       array->array[iterator] = item;
       while (iterator < array->size)
       {
           iterator++;
           type_t tmp2 = array->array[iterator];
           array->array[iterator] = tmp;
           tmp = tmp2;
       }
       array->size++;
   }
}

type_t lookup_ordered_array(uint32 i, ordered_array_t *array)
{
   ASSERT(i < array->size);
   return array->array[i];
}

void remove_ordered_array(uint32 i, ordered_array_t *array)
{
   ASSERT(i < array->size);
   while (i < array->size - 1) 
   {
       array->array[i] = array->array[i+1];
       i++;
   }
   array->size--;
}

uint32 search_ordered_array(type_t item, ordered_array_t *array)
{   
    uint32 index = -1;
    for(uint32 i=0; i<array->size; i++)
    {
        if(array->array[i] == item)
        {
            index = i;
            break;
        }
    }
    return index;
}