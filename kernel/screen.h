#include "types.h"
#define VIDEO_MEM 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
#define WHITE_ON_BLACK 0x0f
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

void print_char(int row, int col, uint8 c, uint8 attri);
void print_dec(uint32 dec);
void print_hex(uint32 hex);
void print(char * msg);
void clear_screen();