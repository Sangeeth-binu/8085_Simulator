#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>


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


void main()
{
    printf("perfect");
}