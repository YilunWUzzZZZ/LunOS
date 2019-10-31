#include "types.h"

typedef struct elf_header{
    uint32 magic;
    uint8 elf[12];
    uint16 type;
    uint16 machine;
    uint32 version;
    uint32 entry;
    uint32 phoff;
    uint32 shoff;
    uint32 flags;
    uint16 ehsize;
    uint16 phentsize;
    uint16 phnum;
    uint16 shentsize;
    uint16 shnum;
    uint16 shstrndx;
} elf_header_t;

typedef struct prog_header{
    uint32 type;
    uint32 off;
    uint32 vaddr;
    uint32 paddr;
    uint32 filesz;
    uint32 memsz;
    uint32 flags;
    uint32 align;
} prog_header_t;

