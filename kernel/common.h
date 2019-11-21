#ifndef COMMON_H
#define COMMON_H
#define ASSERT(expr) ( (expr)? 0 : panic_assert(#expr) )

void panic(char * msg);
void panic_assert(char * msg);

#endif