#ifndef KHEAP_H
#define KHEAP_H

#include "ordered_list.h"

#define KHEAP_START 0xE0000000
#define KHEAP_MAX 0xE8000000
#define KHEAP_INITIAL_SIZE 0x100000
#define HEAP_INDEX_SIZE 0x20000
#define HEAP_MAGIC 0x19980603


typedef struct 
{
    uint32 magic;
    uint8 is_hole; //not used?
    uint32 size; //= header size + footer size + usable size
} header_t;

typedef struct 
{
    uint32 magic;
    header_t * header;
} footer_t;

typedef struct 
{
    ordered_array_t index;
    uint32 start_address; // The start of our allocated space.
    uint32 end_address;   // The end of our allocated space. May be expanded up to max_address.
    uint32 max_address;   // The maximum address the heap can be expanded to.
    uint8 supervisor;     // Should extra pages requested by us be mapped as supervisor-only?
    uint8 readonly;       // Should extra pages requested by us be mapped as read-only?
} heap_t;

uint32 kmalloc(uint32 size, char align);
heap_t *create_heap(uint32 start, uint32 end, uint32 max, uint8 supervisor, uint8 readonly);
void *alloc(uint32 size, uint8 page_align, heap_t *heap);
void free(void * address, heap_t * heap);
#endif