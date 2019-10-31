#include "screen.h"
#include "utils.h"

static inline int  coord_to_offset(int row, int col){
    return (row * 80 + col) * 2;
   
}

int get_cursor(){
    outb(REG_SCREEN_CTRL, 14);
    int offset = inb(REG_SCREEN_DATA) << 8;
    outb(REG_SCREEN_CTRL, 15);
    offset += inb(REG_SCREEN_DATA);
    return offset * 2;
}

int set_cursor(int offset){
    offset /= 2;
    outb(REG_SCREEN_CTRL, 14);
    outb(REG_SCREEN_DATA, (uint8)(offset >> 8));
    outb(REG_SCREEN_CTRL, 15);
    outb(REG_SCREEN_DATA, (uint8)offset);
}

int handle_scrolling(int offset){
    if(offset < MAX_COLS * MAX_ROWS * 2){
        return offset;
    }
    char * video = (char *)VIDEO_MEM;
    for(int i=1; i<MAX_ROWS; i++){
        uint8 * dest = video + coord_to_offset(i-1, 0);
        uint8 * src = video + coord_to_offset(i, 0);
        memcpy(dest, src, MAX_COLS * 2);
    }
    char * lastline = video + (MAX_ROWS-1)*MAX_COLS*2;
    memset((uint8*)lastline, 0, MAX_COLS);
    return offset - MAX_COLS *2;

}


void print_char(int row, int col, uint8 c, uint8 attri){
    if(!attri){
        attri = WHITE_ON_BLACK;
    }
    // if coordinate out of bounds, set to pos of the cursor;
    int offset;
    if(col >=0 && row >= 0 && col < MAX_COLS && row < MAX_ROWS){
        offset = coord_to_offset(row, col);
    }
    else{
        offset = get_cursor();
    }
    uint8 * video = (uint8 *)VIDEO_MEM;
    
    if(c == '\n'){
        row = offset /(MAX_COLS * 2);
        offset = coord_to_offset(row, 79);
    }
    else{
        video[offset] = c;
        video[offset+1] = attri;
    }

    offset += 2;

    offset = handle_scrolling(offset);

    set_cursor(offset);
}

void print_at(int row, int col, char * msg, uint8 attri){
    int i = 0;

    if(col >=0 && row >= 0 && col < MAX_COLS && row < MAX_ROWS){
        set_cursor(coord_to_offset(row, col));
    }

    while (msg[i] != 0)
    {
        print_char(-1, -1, msg[i++], attri);
    }
    
}

void print_dec(uint32 dec){
    uint8 digits[15];
    uint8 cnt=0;
    do{
        digits[cnt++] = dec%10;
        dec = dec/10;

    }while(dec);
    for(int i=cnt-1; i>=0; i--){
        print_char(-1,-1,digits[i]+48,0);
    }
}

void print_hex(uint32 hex){
    uint8 digits[15];
    uint8 cnt=0;
    do{
        digits[cnt++] = hex%16;
        hex = hex/16;

    }while(hex);
    for(int i=cnt-1; i>=0; i--){

        if(0 <= digits[i] && digits[i] <= 9){
            print_char(-1,-1,digits[i]+48,0);
        }
        else{
            print_char(-1,-1,digits[i]+55, 0);
        }
        
    }
}

void print(char * msg){
    print_at(-1, -1, msg, 0);
}

void clear_screen(){
    for(int i=0; i<MAX_ROWS; i++){
        for(int j=0; j<MAX_COLS; j++){
            print_char(i,j,' ', 0);
        }
    }
    set_cursor(0);
}
