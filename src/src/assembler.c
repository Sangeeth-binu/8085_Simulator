#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

static FILE *assembly;
static uint8_t input_buffer_read_count = 128;
static int input_buffer[128];

void output_1c(uint8_t b)
{
    printf("%c", b);
}
void fatal(char *s)
{
    uint8_t b;
    b = *s++;
    while(b)
    {
        output_1c(b);
        if(!b) return;
        b = *s++;
    }
}
void print_str(char *s, unsigned l)
{
    uint8_t b;
    while(l--){
        b = *s++;
        if(!b) return;//stop at null
        output_1c(b);
    }
}
void print(char *s)
{
    uint8_t b;
    while(1)
    {
        b = *s++;
        if(!b) return;
        output_1c(b);

    }
}
void println(char *s)
{
    print(s);
    printf("\n");
}
void print_hex4(uint8_t nibble)
{
    printf("%01x", nibble);
}
void print_hex8(uint8_t b)
{
    print_hex4(b>>4);
    print_hex4(b);
}
void print_hex16(uint16_t v)
{
    print_hex8(v>>8);
    print_hex8(v);
}
void printi(int i)
{
    char c;
    c = (char)i;
    output_1c(c);
}
enum{
    TOKEN_EOF = EOF,
    TOKEN_NEWLINE = '\n',
    TOKEN_STRING = 3,
    TOKEN_IDENTIFIER = 2,
    TOKEN_NUMBER = 1
};

struct output_file
{
    FILE* fp;
    uint8_t count;
    uint8_t buffer[128];
    bool opened:1;
};

struct symbols
{
    char* name;
    void (*callback)(void);
    struct symbols* next;
    uint8_t length;
    uint16_t value;
};

int read_byte(){
    int b;
    uint8_t flag =0;
    if(input_buffer_read_count == 128){//buffer is read successful
        uint8_t i =0;
        b=fgetc(assembly);
        while(1){
    
            if(b==EOF){
                flag = 1;
            }
            if(flag){
                input_buffer[i] = EOF;
                i++;
            }else{
                input_buffer[i] = b;
                i++;
            }
            if(i>=128){
                break;
            }
            b=fgetc(assembly);
        }
        input_buffer_read_count=0;
    }
    b = input_buffer[input_buffer_read_count++];
    return b;
}
void main()
{
    assembly = fopen("test.asm","r");

    if(assembly == NULL){
        fatal("error opening file");
        exit(1);
    }
    for(int i =0;i<1000;i++){
        printi(read_byte());
    }
    fclose(assembly);
    return;
}