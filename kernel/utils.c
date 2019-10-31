#include "types.h"

void memcpy(uint8 * dest, uint8 * src, uint32 count){
    for(uint32 i=0; i<count; i++){
        dest[i] = src[i];
    }
}

void memset(sint8 * begin, sint8 val, uint32 count){
    for(int i=0; i< count; i++){
        begin[i] = val;
    }
}