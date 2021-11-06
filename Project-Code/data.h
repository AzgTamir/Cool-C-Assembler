
#ifndef _DATA_H_
#define _DATA_H_

#include "data_defs.h"
#include "token.h"
/* 
data.h:
this file contains struct and enum that are being used all over the program
*/

/**
 * @brief enum to represent the ARE attribute in the decimal value
 * 
 */
enum 
{
    ABSOLUTE_ARE = 4,
    RELOCAL_ARE = 2,
    EXTERNAL_ARE = 1,
    UNDEFINED_ARE = 0
};
/**
 * @brief bit structure of the data word
 * 
 */
typedef struct bin_arg_t 
{
    unsigned int ARE : 3;
    int value : 21;
} bin_arg_t;
/**
 * @brief holds a 24 bit the represent the value
 * 
 */
typedef struct bin_dir_t
{
    int value : 24;
} bin_dir_t;

/**
 * @brief the 24bit divided like shows in the table at page 21 in the course workbook
 * 
 */
typedef struct bin_op_t
{
    unsigned int ARE : 3;/* bit 0 -> 2, E => 0, R => 1, A => 2 */
    unsigned int func : 5;/* bit 3 -> 7 */

    unsigned int dst_r : 3;/* bit 8 -> 10 */
    unsigned int dst : 2;/* bit 11 -> 12 */
    unsigned int src_r : 3;/* bit 13 -> 15 */

    unsigned int src : 2;/* bit 16 -> 17 */
    unsigned int op : 6;/* bit 18 -> 23 */
    /* a total of 24 bit field */
} bin_op_t;
/**
 * @brief a union that all the difrent bit fields struct pointing at the same 24bit field
 * 
 */
typedef union bin_cmd_t
{
    bin_dir_t _bin_dir;/*  a full 24 bit of int */
    bin_arg_t _bin_arg;/* data word and ARE */
    bin_op_t _bin_op;/* coding in format of the table in page 21  */
    unsigned char _bin_cmd[3]; /* 3byte = 24 bit */
} bin_cmd_t;

/**
 * @brief asm data with binary cmd 
 *  the token "class" holding all the information and proprties of the token
 */
typedef struct asm_cmd_t
{
    bin_cmd_t _binCmd; /* binary code */
    type_t _type;/* the word type a.k.a token , an enum value*/
    char _labelStr[SYM_NAME_LEN];/* char* version of the word */
    unsigned int _line; /* line number */
} asm_cmd_t;

extern asm_cmd_t *asmCmd;

/**
 * @brief enum that symbolise the coding method of the data word
 * 
 */
typedef enum {
    SYM_LOCAL,
    SYM_ENTRY,
    SYM_EXTERN,
    SYM_UNDEFINED
} sym_attr_t;

/**
 * @brief enum of every instruction op code
 * 
 */
typedef enum
{
    MOV = 0,
    CMP,
    ADD,
    SUB,
    LEA,
    CLR,
    NOT,
    INC,
    DEC,
    JMP,
    BNE,
    JSR,
    RED,
    PRN,
    RTS,
    STOP,
    OPS_NUM
} op_t;

/**
 * @brief enum that symbolise the funct value
 * 
 */
typedef enum
{
    OP_MOV = 0,
    OP_CMP = 1,
    OP_ADD_SUB = 2,
    OP_LEA = 4,
    OP_CLR_NOT_INC_DEC = 5,
    OP_JMP_BNE_JSR = 9,
    OP_RED = 12,
    OP_PRN = 13,
    OP_RTS = 14,
    OP_STOP = 15,
    OP_NULL = -1
} op_num_t;


/**
 * @brief holder of command information such as
 *       opartion code, name(char*), funct, amount of args, 
 *       all the options for the types of surce parameter,
 *       all the options for the types of the destantion parameter
 * 
 */
typedef struct cmd_ref_table 
{
    char str[10];
    op_num_t opNum;
    int funcNum;
    int argsNum;
    type_t srcArg[4]; /*the type of argument the current operand acceptes (in source) */
    type_t dstArg[4]; /*the type of argument the current operand acceptes (in desdenation) 
                        the array is null-terminated. e.g: current op: CLR => srcArg = {NUL_TOK} 
                        , dstArg={LABEL_ARG, REG_ARG, NUM_ARG, NUL_TOK} */
} cmd_ref_table;

extern const cmd_ref_table op_table[]; /* the table that contains info on every instruction information - initialized in token.c */

/**
 * @brief represent single label and his attribute
 * 
 */
typedef struct sym_table_t
{
    char symStr[SYM_NAME_LEN];
    unsigned int addr;
    unsigned int line;
    sym_attr_t LabelAttr;
} sym_table_t;

extern sym_table_t *labelTable;/* global array of labels */

#endif /* _DATA_H_ */