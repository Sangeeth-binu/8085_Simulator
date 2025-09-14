#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#define NL printf("\n")

static FILE *assembly;
bool read = false;
static uint8_t token_buffer[64];
static uint8_t token_length;
static int token_number;
static unsigned int line_number = 1;
bool end_of_line = false;

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


enum
{
    TOKEN_EOF = EOF,
    TOKEN_NEWLINE = '\n',
    TOKEN_STRING = 3,
    TOKEN_IDENTIFIER = 2,
    TOKEN_NUMBER = 1,
    TOKEN_COMMA = ',',
    TOKEN_COLON = ':',
    TOKEN_LABEL = 4
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

int read_byte()
{
    return fgetc(assembly);
}
void unread_byte()
{
   fseek(assembly,-1,SEEK_CUR);
}

bool is_ident(uint8_t c)
{
    return isalnum(c)||(c=='_');
}
void check_token_buffer_size()
{
    if(token_length >= 64)
    {
        fatal("\n error: buffer overflow");
        fclose(assembly);
        exit(1);
    }
}

void init_token_buffer()
{
    for(uint8_t i =0;i<64;i++){
        token_buffer[i]=0;
    }
}

int read_token()
{   read = false;
    init_token_buffer();
    int c;
    do{
        token_length = 0;
        c = read_byte();
        if(c == EOF){
            return TOKEN_EOF;
        }
        if(c == '\n'){
            end_of_line = true;
        }
        
        if(c == ' ' || c == '\t'){ continue;}

        if(c==';')
        {
            do{
                c = read_byte();
            }while(c!='\n' && c!=EOF);
            if(c=='\n'){
                end_of_line = true;
            }
        }
        if(end_of_line)
        {
           ++line_number;
           end_of_line = false;
        }
    }while(c == ' '|| c =='\t' || c==';');
    //getting started

    c = toupper(c);                                                 //converting to upper case
    token_length =0;                                               //setting token buffer to receive the input
    if(isdigit(c))
    {
      
        uint8_t base;
        //filling the token
        while(isalnum(c)){
            check_token_buffer_size();                              //safety first
            token_buffer[token_length++] = c;
            c = toupper(read_byte());
        }
        unread_byte();

        base = 10;                                                //decimal in default
        c = token_buffer[--token_length];                           // finding the last element to check the base of number 
    
        switch(c)
        {
            case 'B':   base = 2;
                        for(uint8_t i =0; i<token_length;i++){
                            if((token_buffer[i] == '1'  )||( token_buffer[i] == '0') ) 
                                    continue;
                            else{
                                fatal("error : not binary");
                                NL;
                                fclose(assembly);
                                exit(1);
                            }
                        }
                        break;

            case 'O':case 'Q':  base = 8;
                                for(uint8_t i =0; i<token_length;i++){
                                if((token_buffer[i] == '0'  )||( token_buffer[i] == '1')||(token_buffer[i] == '2' )||( token_buffer[i] == '3')||(token_buffer[i] == '4'  )||( token_buffer[i] == '5')||(token_buffer[i] == '6' )||( token_buffer[i] == '7') ){
                                    continue;
                                }else{
                                    fatal("error : not octal");
                                    NL;
                                    fclose(assembly);
                                    exit(1);
                                }
                                }
                                break;

            case 'H':   base = 16;
                        for(uint8_t i =0; i<token_length;i++){
                            if((token_buffer[i] == '0'  )||( token_buffer[i] == '1')||(token_buffer[i] == '2' )||( token_buffer[i] == '3')||(token_buffer[i] == '4'  )||( token_buffer[i] == '5')||(token_buffer[i] == '6' )||( token_buffer[i] == '7')||( token_buffer[i] == '8')||(token_buffer[i] == '9' )||( token_buffer[i] == 'A')||(token_buffer[i] == 'B'  )||( token_buffer[i] == 'C')||(token_buffer[i] == 'D' )||( token_buffer[i] == 'E')||( token_buffer[i] == 'F')){
                                continue;
                            }else{
                                fatal("error : not hex");
                                NL;
                                fclose(assembly);
                                exit(1);
                        }
                        break;
            case 'D': base = 10;break;            
            default : if(isdigit(c)){ token_length++; }
        }

        token_number = 0;
        for(uint8_t i =0;i<token_length;i++)
        {
            c = token_buffer[i];                        
            if(c >= 'A'){ c = c-'A'+10;}else{c=c-'0';}                 //converting to decimal
            token_number =(token_number*base) + c; 
        }
        return TOKEN_NUMBER;  
        }
    }else if(isupper(c)){
        while(1)
        {
            check_token_buffer_size();
            token_buffer[token_length++] = c;
            c = toupper(read_byte());

            if(!is_ident(c)){
                if(c==':'){
                    return TOKEN_LABEL;
                }
                break;
            }
        }
        unread_byte();

        return TOKEN_IDENTIFIER;
    }else if(c == '\''){
        uint8_t string_length = 0;
        while(1){
            c  = read_byte();
            check_token_buffer_size();
            token_buffer[token_length++]=c;
            if(c=='\''){
                token_length--;
                token_buffer[token_length]='\0';
                break;
            }
            string_length++;
            if(string_length >=63){
                fatal("string length exceed");
                break;
            }
        }
        return TOKEN_STRING;
    }else if(c==','){
        return TOKEN_COMMA;
    }else if(c=='\n'){
        return TOKEN_NEWLINE;
    }else if(c==0){
        return TOKEN_EOF;
    }else if(c==':')
    return TOKEN_EOF;

} 

void print_token_info(int token_type){
    printf(" token type : %d", token_type);
    switch(token_type){
        case TOKEN_NUMBER:
            printf(" (NUMBER) Value: %d", token_number);
            break;
        case TOKEN_IDENTIFIER:
            printf(" (IDENTIFIER) Text: ");
            for(int i = 0; i < token_length; i++) {
                printf("%c", token_buffer[i]);
            }
            break;
        case TOKEN_STRING:
            printf(" (STRING) Text: ");
            for(int i = 0; i < token_length; i++) {
                printf("%c", token_buffer[i]);
            }
            break;
        case TOKEN_COMMA:
            printf(" (COMMA)");
            break;
        case TOKEN_NEWLINE:
            printf(" (NEWLINE)");
            break;
        case TOKEN_LABEL:
            printf(" (LABEL)");
            break;
        case TOKEN_COLON:
            printf(" (COLON)");
            break;
        case TOKEN_EOF:
            printf(" (EOF)");
            break;
    }
    NL;
}


void main()
{
    assembly = fopen("test.asm","r");

    if(assembly == NULL){
        fatal("error opening file");
        exit(1);
    }
    int token_type;
    int count =0;
    printf("\n===== Token test =====\n");
    do{
        token_type = read_token();
        printf("Token %d: ", ++count);
        print_token_info(token_type);

        if(count > 25){
            printf("\nstopping after 25 tokens \n");
            break;
        }
    }while(token_type != TOKEN_EOF);

    fclose(assembly);
    return;
}