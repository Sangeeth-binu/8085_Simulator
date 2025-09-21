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
int slot;

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
    OP_ADD,
    OP_AND,
    OP_DIV,
    OP_MOD,
    OP_MUL,
    OP_NEG,
    OP_NOT,
    OP_OR,
    OP_SHL,
    OP_SHR,
    OP_SUB,
    OP_XOR,
    OP_PAR,
    OP_SET
};

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

struct symbol
{
    char* name;                 //name of instruction
    uint8_t length;             //length of the name
    uint16_t value;             //opcode/address/
    void (*call_back)(void);     //function to handle that symbol
    struct symbol* next;       //pointer to next sybol in the linked list
};

struct symbol* token_symbol;

// Basic callback functions
void equlabel_callback(void) { 
    printf("equlabel_callback called\n"); 
}

void operator_callback(void) { 
    printf("operator_callback called\n"); 
}

// A instructions
void handle_adc(void) { printf("ADC instruction\n"); }
void handle_add(void) { printf("ADD instruction\n"); }
void handle_aci(void) { printf("ACI instruction\n"); }
void handle_adi(void) { printf("ADI instruction\n"); }
void handle_ana(void) { printf("ANA instruction\n"); }
void handle_ani(void) { printf("ANI instruction\n"); }
void handle_and(void) { printf("AND operator\n"); }

// C instructions
void handle_call(void) { printf("CALL instruction\n"); }
void handle_cc(void) { printf("CC instruction\n"); }
void handle_cm(void) { printf("CM instruction\n"); }
void handle_cma(void) { printf("CMA instruction\n"); }
void handle_cmc(void) { printf("CMC instruction\n"); }
void handle_cmp(void) { printf("CMP instruction\n"); }
void handle_cnc(void) { printf("CNC instruction\n"); }
void handle_cnz(void) { printf("CNZ instruction\n"); }
void handle_cp(void) { printf("CP instruction\n"); }
void handle_cpe(void) { printf("CPE instruction\n"); }
void handle_cpi(void) { printf("CPI instruction\n"); }
void handle_cpo(void) { printf("CPO instruction\n"); }
void handle_cz(void) { printf("CZ instruction\n"); }

// D instructions
void handle_daa(void) { printf("DAA instruction\n"); }
void handle_dad(void) { printf("DAD instruction\n"); }
void handle_dcr(void) { printf("DCR instruction\n"); }
void handle_dcx(void) { printf("DCX instruction\n"); }
void handle_di(void) { printf("DI instruction\n"); }
void handle_db(void) { printf("DB directive\n"); }
void handle_ds(void) { printf("DS directive\n"); }
void handle_dw(void) { printf("DW directive\n"); }

// E instructions
void handle_ei(void) { printf("EI instruction\n"); }
void handle_else(void) { printf("ELSE directive\n"); }
void handle_end(void) { printf("END directive\n"); }
void handle_endif(void) { printf("ENDIF directive\n"); }
void handle_equ(void) { printf("EQU directive\n"); }

// H instructions
void handle_hlt(void) { printf("HLT instruction\n"); }

// I instructions
void handle_in(void) { printf("IN instruction\n"); }
void handle_inr(void) { printf("INR instruction\n"); }
void handle_inx(void) { printf("INX instruction\n"); }
void handle_if(void) { printf("IF directive\n"); }

// J instructions
void handle_jc(void) { printf("JC instruction\n"); }
void handle_jm(void) { printf("JM instruction\n"); }
void handle_jmp(void) { printf("JMP instruction\n"); }
void handle_jnc(void) { printf("JNC instruction\n"); }
void handle_jnz(void) { printf("JNZ instruction\n"); }
void handle_jp(void) { printf("JP instruction\n"); }
void handle_jz(void) { printf("JZ instruction\n"); }
void handle_jpo(void) { printf("JPO instruction\n"); }
void handle_jpe(void) { printf("JPE instruction\n"); }

// L instructions
void handle_lxi(void) { printf("LXI instruction\n"); }
void handle_lhld(void) { printf("LHLD instruction\n"); }
void handle_ldax(void) { printf("LDAX instruction\n"); }
void handle_lda(void) { printf("LDA instruction\n"); }

// M instructions
void handle_mov(void) { printf("MOV instruction\n"); }
void handle_mvi(void) { printf("MVI instruction\n"); }
void handle_mod(void) { printf("MOD operator\n"); }

// N instructions
void handle_nop(void) { printf("NOP instruction\n"); }
void handle_not(void) { printf("NOT operator\n"); }

// O instructions
void handle_org(void) { printf("ORG directive\n"); }
void handle_ora(void) { printf("ORA instruction\n"); }
void handle_ori(void) { printf("ORI instruction\n"); }
void handle_out(void) { printf("OUT instruction\n"); }
void handle_or(void) { printf("OR operator\n"); }

// P instructions
void handle_pchl(void) { printf("PCHL instruction\n"); }
void handle_pop(void) { printf("POP instruction\n"); }
void handle_push(void) { printf("PUSH instruction\n"); }

// R instructions
void handle_ral(void) { printf("RAL instruction\n"); }
void handle_rar(void) { printf("RAR instruction\n"); }
void handle_rc(void) { printf("RC instruction\n"); }
void handle_ret(void) { printf("RET instruction\n"); }
void handle_rlc(void) { printf("RLC instruction\n"); }
void handle_rm(void) { printf("RM instruction\n"); }
void handle_rnc(void) { printf("RNC instruction\n"); }
void handle_rnz(void) { printf("RNZ instruction\n"); }
void handle_rp(void) { printf("RP instruction\n"); }
void handle_rpe(void) { printf("RPE instruction\n"); }
void handle_rpo(void) { printf("RPO instruction\n"); }
void handle_rrc(void) { printf("RRC instruction\n"); }
void handle_rst(void) { printf("RST instruction\n"); }
void handle_rz(void) { printf("RZ instruction\n"); }

// S instructions
void handle_set(void) { printf("SET directive\n"); }
void handle_sbb(void) { printf("SBB instruction\n"); }
void handle_sbi(void) { printf("SBI instruction\n"); }
void handle_shld(void) { printf("SHLD instruction\n"); }
void handle_sphl(void) { printf("SPHL instruction\n"); }
void handle_sta(void) { printf("STA instruction\n"); }
void handle_stax(void) { printf("STAX instruction\n"); }
void handle_stc(void) { printf("STC instruction\n"); }
void handle_sub(void) { printf("SUB instruction\n"); }
void handle_sui(void) { printf("SUI instruction\n"); }

// X instructions
void handle_xthl(void) { printf("XTHL instruction\n"); }
void handle_xri(void) { printf("XRI instruction\n"); }
void handle_xra(void) { printf("XRA instruction\n"); }
void handle_xchg(void) { printf("XCHG instruction\n"); }

#define INSTRUCTION(id,name,value,call_back,next)\
    extern void call_back(void);\
    struct symbol id = {name,sizeof(name)-1,value,call_back,next};
#define VALUE(id,name,value,next)\
    struct symbol id = {name,sizeof(name)-1,value,equlabel_callback,next};

// 8085 Instruction Set

// A
VALUE(reg_a_sym, "A", 0x07, NULL)                             // Accumulator
INSTRUCTION(adc_sym, "ADC", 0x88, handle_adc, &reg_a_sym)     // Base: 10001rrr
INSTRUCTION(add_sym, "ADD", 0x80, handle_add, &adc_sym)       // Base: 10000rrr
INSTRUCTION(aci_sym, "ACI", 0xCE, handle_aci, &add_sym)       // Add immediate with carry
INSTRUCTION(adi_sym, "ADI", 0xC6, handle_adi, &aci_sym)       // Add immediate
INSTRUCTION(ana_sym, "ANA", 0xA0, handle_ana, &adi_sym)       // Base: 10100rrr
INSTRUCTION(ani_sym, "ANI", 0xE6, handle_ani, &ana_sym)       // AND immediate
INSTRUCTION(and_sym, "AND", 0x00, handle_and, &ani_sym)       // Logical AND operator
//B
VALUE(reg_b_sym, "B", 0x00, NULL)                             // Register B
VALUE(reg_bc_sym, "BC", 0x00, &reg_b_sym)                     // BC pair

// C
VALUE(reg_c_sym, "C", 0x01, NULL)                             // Register C  
INSTRUCTION(call_sym, "CALL", 0xCD, handle_call, &reg_c_sym)  // Call unconditional
INSTRUCTION(cc_sym, "CC", 0xDC, handle_cc, &call_sym)         // Call if carry
INSTRUCTION(cm_sym, "CM", 0xFC, handle_cm, &cc_sym)           // Call if minus
INSTRUCTION(cma_sym, "CMA", 0x2F, handle_cma, &cm_sym)        // Complement A
INSTRUCTION(cmc_sym, "CMC", 0x3F, handle_cmc, &cma_sym)       // Complement carry
INSTRUCTION(cmp_sym, "CMP", 0xB8, handle_cmp, &cmc_sym)       // Base: 10111rrr
INSTRUCTION(cnc_sym, "CNC", 0xD4, handle_cnc, &cmp_sym)       // Call if no carry
INSTRUCTION(cnz_sym, "CNZ", 0xC4, handle_cnz, &cnc_sym)       // Call if not zero
INSTRUCTION(cp_sym, "CP", 0xF4, handle_cp, &cnz_sym)          // Call if positive
INSTRUCTION(cpe_sym, "CPE", 0xEC, handle_cpe, &cp_sym)        // Call if parity even
INSTRUCTION(cpi_sym, "CPI", 0xFE, handle_cpi, &cpe_sym)       // Compare immediate
INSTRUCTION(cpo_sym, "CPO", 0xE4, handle_cpo, &cpi_sym)       // Call if parity odd
INSTRUCTION(cz_sym, "CZ", 0xCC, handle_cz, &cpo_sym)          // Call if zero

// D
VALUE(reg_d_sym, "D", 0x02, NULL)                             // Register D
VALUE(reg_de_sym, "DE", 0x01, &reg_d_sym)                     // DE pair
INSTRUCTION(daa_sym, "DAA", 0x27, handle_daa, &reg_de_sym)
INSTRUCTION(dad_sym, "DAD", 0x09, handle_dad, &daa_sym)       // Base: 00rp1001
INSTRUCTION(dcr_sym, "DCR", 0x05, handle_dcr, &dad_sym)       // Base: 00rrr101
INSTRUCTION(dcx_sym, "DCX", 0x0B, handle_dcx, &dcr_sym)       // Base: 00rp1011
INSTRUCTION(di_sym, "DI", 0xF3, handle_di, &dcx_sym)          // Disable interrupts
INSTRUCTION(db_sym, "DB", 0x00, handle_db, &di_sym)           // Define Byte
INSTRUCTION(ds_sym, "DS", 0x00, handle_ds, &db_sym)           // Define Storage  
INSTRUCTION(dw_sym, "DW", 0x00, handle_dw, &ds_sym)           // Define Word

// E
VALUE(reg_e_sym, "E", 0x03, NULL)                             // Register E
INSTRUCTION(ei_sym, "EI", 0xFB, handle_ei, &reg_e_sym)        // Enable interrupts
INSTRUCTION(else_sym, "ELSE", 0x00, handle_else, &ei_sym)     // Conditional assembly
INSTRUCTION(end_sym, "END", 0x00, handle_end, &else_sym)      // End of program
INSTRUCTION(endif_sym, "ENDIF",0x00, handle_endif, &end_sym)  // End conditional
INSTRUCTION(equ_sym, "EQU", 0x00, handle_equ, &endif_sym)     // Equate symbol


// H
VALUE(reg_h_sym, "H", 0x04, NULL)                             // Register H
VALUE(reg_hl_sym, "HL", 0x02, &reg_h_sym)                     // HL pair  
INSTRUCTION(hlt_sym, "HLT", 0x76, handle_hlt, &reg_hl_sym)    // Halt

// I
INSTRUCTION(in_sym, "IN", 0xDB, handle_in, NULL)              // Input from port
INSTRUCTION(inr_sym, "INR", 0x04, handle_inr, &in_sym)        // Base: 00rrr100
INSTRUCTION(inx_sym, "INX", 0x03, handle_inx, &inr_sym)       // Base: 00rp0011
INSTRUCTION(if_sym, "IF", 0x00, handle_if, &inx_sym)          // Conditional assembly

// J
INSTRUCTION(jc_sym, "JC", 0xDA, handle_jc, NULL)              // Jump if carry
INSTRUCTION(jm_sym, "JM", 0xFA, handle_jm, &jc_sym)           // Jump if minus
INSTRUCTION(jmp_sym, "JMP", 0xC3, handle_jmp, &jm_sym)        // Jump unconditional
INSTRUCTION(jnc_sym, "JNC", 0xD2, handle_jnc, &jmp_sym)       // Jump if no carry
INSTRUCTION(jnz_sym, "JNZ", 0xC2, handle_jnz, &jnc_sym)       // Jump if not zero
INSTRUCTION(jp_sym, "JP", 0xF2, handle_jp, &jnz_sym)          // Jump if positive
INSTRUCTION(jz_sym, "JZ", 0xCA, handle_jz, &jp_sym)           // Jump if zero
INSTRUCTION(jpo_sym, "JPO", 0xE2, handle_jpo, &jz_sym)        // Jump if parity odd
INSTRUCTION(jpe_sym, "JPE", 0xEA, handle_jpe, &jpo_sym)       // Jump if parity even



// L
VALUE(reg_l_sym, "L", 0x05, NULL)                             // Register L
INSTRUCTION(lxi_sym, "LXI", 0x01, handle_lxi, &reg_l_sym)     // Base: 00rp0001
INSTRUCTION(lhld_sym, "LHLD", 0x2A, handle_lhld, &lxi_sym)    // Load HL direct
INSTRUCTION(ldax_sym, "LDAX", 0x0A, handle_ldax, &lhld_sym)   // Base: 00rp1010
INSTRUCTION(lda_sym, "LDA", 0x3A, handle_lda, &ldax_sym)      // Load A direct




// M
VALUE(reg_m_sym, "M", 0x06, NULL)    // Memory (HL)
INSTRUCTION(mov_sym, "MOV", 0x40, handle_mov, &reg_m_sym)     // Base: 01dddss
INSTRUCTION(mvi_sym, "MVI", 0x06, handle_mvi, &mov_sym)       // Base: 00ddd110
INSTRUCTION(mod_sym, "MOD", 0x00, handle_mod, &mvi_sym)       // Modulo operator

// N
INSTRUCTION(nop_sym, "NOP", 0x00, handle_nop, NULL)           // No operation
INSTRUCTION(not_sym, "NOT", 0x00, handle_not, &nop_sym)       // Logical NOT operator

// O
INSTRUCTION(org_sym, "ORG", 0x00, handle_org, NULL)           // Origin/Start address
INSTRUCTION(ora_sym, "ORA", 0xB0, handle_ora, &org_sym)       // Base: 10110rrr
INSTRUCTION(ori_sym, "ORI", 0xF6, handle_ori, &ora_sym)       // OR immediate
INSTRUCTION(out_sym, "OUT", 0xD3, handle_out, &ori_sym)       // Output to port
INSTRUCTION(or_sym, "OR", 0x00, handle_or, &out_sym)          // Logical OR operator

// P
INSTRUCTION(pchl_sym, "PCHL", 0xE9, handle_pchl, NULL)
VALUE(reg_psw_sym, "PSW", 0x03, &pchl_sym)                    // PSW (A+Flags) for PUSH/POP
INSTRUCTION(pop_sym, "POP", 0xC1, handle_pop, &reg_psw_sym)   // Base: 11rp0001
INSTRUCTION(push_sym, "PUSH", 0xC5, handle_push, &pop_sym)    // Base: 11rp0101

// R
INSTRUCTION(ral_sym, "RAL", 0x17, handle_ral, NULL)           // Rotate A left through carry
INSTRUCTION(rar_sym, "RAR", 0x1F, handle_rar, &ral_sym)       // Rotate A right through carry
INSTRUCTION(rc_sym, "RC", 0xD8, handle_rc, &rar_sym)          // Return if carry
INSTRUCTION(ret_sym, "RET", 0xC9, handle_ret, &rc_sym)        // Return unconditional
INSTRUCTION(rlc_sym, "RLC", 0x07, handle_rlc, &ret_sym)       // Rotate A left
INSTRUCTION(rm_sym, "RM", 0xF8, handle_rm, &rlc_sym)          // Return if minus
INSTRUCTION(rnc_sym, "RNC", 0xD0, handle_rnc, &rm_sym)        // Return if no carry
INSTRUCTION(rnz_sym, "RNZ", 0xC0, handle_rnz, &rnc_sym)       // Return if not zero
INSTRUCTION(rp_sym, "RP", 0xF0, handle_rp, &rnz_sym)          // Return if positive
INSTRUCTION(rpe_sym, "RPE", 0xE8, handle_rpe, &rp_sym)        // Return if parity even
INSTRUCTION(rpo_sym, "RPO", 0xE0, handle_rpo, &rpe_sym)       // Return if parity odd
INSTRUCTION(rrc_sym, "RRC", 0x0F, handle_rrc, &rpo_sym)       // Rotate A right
INSTRUCTION(rst_sym, "RST", 0xC7, handle_rst, &rrc_sym)       // Base: 11nnn111
INSTRUCTION(rst0_sym, "RST 0", 0xC7, handle_rst, &rst_sym)    // RST 0
INSTRUCTION(rst1_sym, "RST 1", 0xCF, handle_rst, &rst0_sym)   // RST 1
INSTRUCTION(rst2_sym, "RST 2", 0xD7, handle_rst, &rst1_sym)   // RST 2
INSTRUCTION(rst3_sym, "RST 3", 0xDF, handle_rst, &rst2_sym)   // RST 3
INSTRUCTION(rst4_sym, "RST 4", 0xE7, handle_rst, &rst3_sym)   // RST 4
INSTRUCTION(rst5_sym, "RST 5", 0xEF, handle_rst, &rst4_sym)   // RST 5
INSTRUCTION(rst6_sym, "RST 6", 0xF7, handle_rst, &rst5_sym)   // RST 6
INSTRUCTION(rst7_sym, "RST 7", 0xFF, handle_rst, &rst6_sym)   // RST 7
INSTRUCTION(rz_sym, "RZ", 0xC8, handle_rz, &rst7_sym)         // Return if zero

// S
VALUE(reg_sp_sym, "SP", 0x03, NULL) // Stack pointer
INSTRUCTION(set_sym, "SET", OP_SET, operator_callback,&reg_sp_sym)    
INSTRUCTION(shl_sym, "SHL", OP_SHL, operator_callback,&set_sym)      
INSTRUCTION(shr_sym, "SHR", 0x00, operator_callback,&shl_sym)
INSTRUCTION(sbb_sym, "SBB", 0x98, handle_sbb, &shr_sym)       // Base: 10011rrr
INSTRUCTION(sbi_sym, "SBI", 0xDE, handle_sbi, &sbb_sym)       // Subtract immediate with borrow
INSTRUCTION(shld_sym, "SHLD", 0x22, handle_shld, &sbi_sym)    // Store HL direct
INSTRUCTION(sphl_sym, "SPHL", 0xF9, handle_sphl, &shld_sym)   // Copy HL to SP
INSTRUCTION(sta_sym, "STA", 0x32, handle_sta, &sphl_sym)      // Store A direct
INSTRUCTION(stax_sym, "STAX", 0x02, handle_stax, &sta_sym)    // Base: 00rp0010
INSTRUCTION(stc_sym, "STC", 0x37, handle_stc, &stax_sym)      // Set carry
INSTRUCTION(sub_sym, "SUB", 0x90, handle_sub, &stc_sym)       // Base: 10010rrr
INSTRUCTION(sui_sym, "SUI", 0xD6, handle_sui, &sub_sym)       // Subtract immediate

// X
INSTRUCTION(xthl_sym, "XTHL", 0xE3, handle_xthl, NULL)        // Exchange top of stack with HL
INSTRUCTION(xri_sym, "XRI", 0xEE, handle_xri, &xthl_sym)      // XOR immediate
INSTRUCTION(xra_sym, "XRA", 0xA8, handle_xra, &xri_sym)       // Base: 10101rrr
INSTRUCTION(xor_sym, "XOR", OP_XOR, operator_callback, &xra_sym) 
INSTRUCTION(xchg_sym, "XCHG", 0xEB, handle_xchg, &xor_sym)    // Exchange HL,DE

struct symbol* hash_table[32]={
    0,
    &and_sym,                   //'A' & 0x1f = 01000001 & 00011111 = 00000001 = 1
    &reg_bc_sym,
    &cz_sym,
    &dw_sym,
    &equ_sym,
    0,
    0,
    &hlt_sym,
    &if_sym,
    &jpe_sym,
    0,
    &lda_sym,
    &mod_sym,
    &not_sym,
    &or_sym,
    &push_sym,
    0,
    &rz_sym,
    &sui_sym,
    0,
    0,
    0,
    0,
    &xchg_sym,
    0,
    0,
    0,
    0,
    0,
    0,
    0
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
        
        //searching for symbols in the symbol able


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

void print_token_info(int token_type)
{
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