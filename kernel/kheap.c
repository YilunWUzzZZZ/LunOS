#include "types.h"
#include "kheap.h"
#include "vm.h"
#include "common.h"


extern uint32 end;
uint32 placement_address = (uint32)&end;
heap_t * kheap = 0;
extern pde_t * kernel_pgdir;

static void set_heap_header(header_t * header, uint8 ishole, uint32 size)
{
    header->is_hole = ishole;
    header->magic = HEAP_MAGIC;
    header->size = size;
}

static void set_heap_footer(footer_t * footer, header_t * header)
{
    
    footer->magic = HEAP_MAGIC;
    footer->header = header;
}

uint32 kmalloc(uint32 size, char align)
{
    
    if(kheap)
    {
        return alloc(size, align, kheap);
    }
    
    
    if(!align)
    {
        uint32 vaddr_ret = placement_address;
        placement_address += size;
        return vaddr_ret;
    }
    else
    {
        uint32 vaddr_ret = PAGEROUNDUP(placement_address);
        placement_address = vaddr_ret + size;
        return vaddr_ret;
    }
}

uint32 kmalloc_p(uint32 size, char align, uint32 * phy_addr)
{
    uint32 vaddr = (uint32)alloc(size, align, kheap);
    *phy_addr = (uint32)get_phy_addr(vaddr, kernel_pgdir);
    return vaddr;
}

void kfree(void * p)
{
    free(p, kheap);
}

static sint32 find_smallest_hole(uint32 size, uint32 page_align, heap_t *heap)
{
   // Find the smallest hole that will fit.
   uint32 iterator = 0;
   while (iterator < heap->index.size)
   {
       header_t *header = (header_t *)lookup_ordered_array(iterator, &heap->index);
       // If the user has requested the memory be page-aligned
       if (page_align > 0)
       {
           // Page-align the starting point of this header.
           uint32 location = (uint32)header;
           sint32 offset = 0;
           if ((location+sizeof(header_t)) & 0xFFF != 0)
               offset = 0x1000 /* page size */  - (location+sizeof(header_t))%0x1000;
           sint32 hole_size = (sint32)header->size - offset;
           // Can we fit now?
           if (hole_size >= (sint32)size)
               break;
       }
       else if (header->size >= size)
           break;
       iterator++;
   }
   // Why did the loop exit?
   if (iterator == heap->index.size)
       return -1; // We got to the end and didn't find anything.
   else
       return iterator;
}

static sint8 header_t_less_than(void*a, void *b)
{
   return (((header_t*)a)->size < ((header_t*)b)->size)?1:0;
}

heap_t *create_heap(uint32 start, uint32 end_addr, uint32 max, uint8 supervisor, uint8 readonly)
{
   heap_t *heap = (heap_t*)kmalloc(sizeof(heap_t), 0);

   // All our assumptions are made on startAddress and endAddress being page-aligned.
   ASSERT(start%0x1000 == 0);
   ASSERT(end_addr%0x1000 == 0);

   // Initialise the index.
   heap->index = place_ordered_array( (void*)start, HEAP_INDEX_SIZE, &header_t_less_than);

   // Shift the start address forward to resemble where we can start putting data.
   start += sizeof(type_t)*HEAP_INDEX_SIZE;

   start = PAGEROUNDUP(start);
   
   // Write the start, end and max addresses into the heap structure.
   heap->start_address = start;
   heap->end_address = end_addr;
   heap->max_address = max;
   heap->supervisor = supervisor;
   heap->readonly = readonly;

   // We start off with one large hole in the index.
   header_t *hole = (header_t *)start;
   hole->size = end_addr-start;
   hole->magic = HEAP_MAGIC;
   hole->is_hole = 1;
   insert_ordered_array((void*)hole, &heap->index);

   footer_t * footer = end_addr - sizeof(footer_t);
   footer->header = hole;
   footer->magic = HEAP_MAGIC;

   return heap;
}

void * alloc(uint32 size, uint8 page_align, heap_t *heap)
{
    uint32 block_size = size + sizeof(header_t) + sizeof(footer_t);
    uint32 iterator = find_smallest_hole(block_size, page_align, heap);
    if(iterator == -1)
    {
        panic("kernel heap exhausted\n");
    }
    header_t * orig_hole = lookup_ordered_array(iterator, &heap->index);
    uint32 orig_hole_start = (uint32)orig_hole + sizeof(header_t);
    uint32 orig_hole_size = orig_hole->size;
    uint32 alloc_block_pos = orig_hole;

    if(page_align && (orig_hole_start & 0xFFF) )
    {
        //do page align
        uint32 offset = 0x1000 - (orig_hole_start % 0x1000);
        alloc_block_pos = orig_hole + offset;
        orig_hole_size -= offset;//reset for convinience of later calculation


        //merge to the left if too small
        if(offset <= sizeof(header_t) + sizeof(footer_t))
        {
            footer_t * left_footer = (footer_t *)((uint32)orig_hole - sizeof(footer_t));
            if(left_footer->magic == HEAP_MAGIC)
            {
                header_t * left_header = left_footer->header;
                left_header->size += offset;

                //move the footer
                left_footer = (footer_t *)(alloc_block_pos - sizeof(footer_t));
                set_heap_footer(left_footer, left_header);

                //if it is a hole, we need to update the list
                if(left_header->is_hole)
                {
                    uint32 i = search_ordered_array(left_header, &heap->index);
                    remove_ordered_array(i, &heap->index);
                    insert_ordered_array(left_header, &heap->index);
                }
            }
            else
            {
                panic("Fatal Mem Leak\n");
            }
            
        }
        //set the left hole
        else
        {
            set_heap_header(orig_hole, 1, offset);
            set_heap_footer((uint32)orig_hole+offset-sizeof(footer_t), orig_hole);
            insert_ordered_array(orig_hole, &heap->index);
        }
        

        uint32 right_hole_size = orig_hole_size - block_size;

        //if room after the allocated block is too small, merge to the block
        if( right_hole_size <= sizeof(header_t) + sizeof(footer_t))
        {
            block_size = block_size + right_hole_size;
        }

    }
     //if room after the allocated block is too small, merge to the block
    else if(orig_hole_size - block_size <= sizeof(header_t) + sizeof(footer_t))
    {
        block_size = orig_hole_size;
    }

    //remove the hole, since it is changed or deleted.
    remove_ordered_array(iterator, &heap->index);

    // if we have free space at the right of the allocated block
    if(orig_hole_size - block_size > 0 )
    {
        header_t * right_hole = (header_t * )(alloc_block_pos + block_size);
        set_heap_header(right_hole, 1, orig_hole_size - block_size);

        footer_t * footer =   (footer_t * ) ( (uint32)right_hole + right_hole->size - sizeof(footer_t) );
        set_heap_footer(footer, right_hole);

        //insert
        insert_ordered_array((void *)right_hole, &heap->index);

    }

    //set the block
    header_t * block_header = (header_t *)alloc_block_pos;
    set_heap_header(block_header, 0, block_size);

    return (void *)(alloc_block_pos + sizeof(header_t));
    
}

void free(void * address, heap_t * heap)
{
    //exit if encountering a null pointer
    if(!address)
        return;

    header_t * header = (header_t *)((uint32)address - sizeof(header_t));
    header_t * footer = (footer_t*)((uint32)header + header->size - sizeof(footer_t));

    ASSERT(header->magic == HEAP_MAGIC && footer->magic == HEAP_MAGIC);

    //return if it is a hole
    if(header->is_hole)
        return;
    else
    {
        header->is_hole = 1; //make it a hole
    }
    

    header_t  * new_header = header;
    footer_t * new_footer = footer;

    //check left
    footer_t * left_footer = (footer_t*)((uint32)header - sizeof(footer_t));
    if(left_footer->magic == HEAP_MAGIC && left_footer->header->is_hole)
    {
        header_t * left_header = left_footer->header;
        left_header->size += header->size;
        new_header =left_header;

        //delete the left hole from the list
        uint32 i =  search_ordered_array(left_header, &heap->index);
        remove_ordered_array(i, &heap->index);
    }

    //check right
    header_t * right_header = (header_t *)((uint32)header + header->size);
    if(right_header->magic == HEAP_MAGIC && right_header->is_hole)
    {
        new_header->size += right_header->size;
        new_footer = (footer_t*)((uint32)right_header + right_header->size - sizeof(footer_t));

        //delete the left hole from the list
        uint32 i =  search_ordered_array(right_header, &heap->index);
        remove_ordered_array(i, &heap->index);
    }

    //link new footer to the new header
    set_heap_footer(new_footer, new_header);
    //insert the new hole
    insert_ordered_array(new_header,&heap->index);
}