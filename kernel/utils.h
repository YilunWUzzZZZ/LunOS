#include "types.h"
#define SECTSIZE 512

// static inline uint8 inb(uint16 port);
// static inline void outb(uint16 port, uint8 data);
// static inline void insl(uint32 port, void * addr, uint32 cnt);
// static inline void stosb(void * addr, uint32 data, int cnt);

static inline uint8 inb(uint16 port){
    uint8 result;
    __asm__ __volatile__("in %1, %0" : "=a"(result) : "d"(port));
    return result;
}

static inline void outb(uint16 port, uint8 data){
    __asm__ __volatile__("out %0, %1" : :"a"(data), "d"(port));
}

static inline void insl(uint32 port, void * addr, uint32 cnt){
    __asm__ __volatile__("cld; rep insl"
            :"=D"(addr), "=c"(cnt)
            :"d"(port), "0"(addr), "1"(cnt)
            :"memory", "cc"
        );
    
}

static inline void stosb(void * addr, uint32 data, int cnt){
    __asm__ __volatile__("cld; rep stosb"
            :"=D"(addr), "=c"(cnt)
            :"0"(addr), "1"(cnt), "a"(data)
            :"memory", "cc"
        );
    
}
void memcpy(uint8 * dest, uint8 * src, uint32 count);
void memset(sint8 * begin, sint8 val, uint32 count);