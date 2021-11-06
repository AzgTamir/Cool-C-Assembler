
#ifndef _TOKEN_H_
#define _TOKEN_H_

#include <stdio.h>

#include "data_defs.h"
typedef enum
{
    FALSE = 0,
    TRUE
} boolean_t;

/*represent kinds of token e.g: add --> OP; .data --> DATA_DIR */
typedef enum
{
    NUL_TOK = -1, 
    UNDEFINED_TOKEN = -1,
    OP = 1, /* operand: mov */
    LABEL_DEF, /* label definition: Hello: */
    LABEL_ADDR, /* label address: &Hello */
    LABEL_ARG, /* label argument: Hello */
    REG_ARG, /* register: r2 */
    NUM_ARG, /* num argument(immediate): #4 */
    DATA_DIR, /*data directive: .data */
    DATA_NUM, /* data number: 4 */
    STRING_DIR, /*string directive: .string */
    STRING_STR, /* e.g "hello" */
    ENTRY_DIR, /* entry directive: .entry */
    EXTERN_DIR, /*extern directive: .extern */
    LINE, /* \n */
    COMMA /* , */
} type_t;

/**
 * @brief represent undefined group tokens 
 * 
 */
enum
{
    UNDEF_LAB = -1,
    UNDEF_REG = -2,
    UNDEF_ARG_REG = -3,
    UNDEF_OP = -4,
    UNDEF_DOT = -5,
    UNDEF_NUM = -6
};

/**
 * @brief represent state of access mode (immedaite, directive, relative, register)
 * 
 */
typedef enum
{
    ACC_IMMDT = 0,
    ACC_DIRECT,
    ACC_RLTV,
    ACC_REG
} accMod_e;

/**
 * @brief represent a singel token
 * 
 */
typedef struct token_t
{
    type_t _type;  /* represent the token's type, can be operand, register and so on. */
    int _subValue; /* can represent the sub value of a token (for register: "r5", sub type will be 5 ) */
    char *str;     /* --> "mov" */
    accMod_e _accMod; /*represnt the access mode of the token */
} token_t;

/**
 * @brief the tokenizer state represent the state of the tokenizer at any given line/token. 
 *        and stores both the entire string line, and token list.
 *        The tokenizerState allow for easy maneuver between any of the functions. 
 * 
 *        *Also contain debuging tool if wanted that print each line's token list and thier attributes.
 * 
 */
typedef struct tokenizerState_t
{
    char *lineWords[MAX_LINE_TOKENS]; /* the given line seperate to words*/
    token_t *tokenList[MAX_LINE_TOKENS]; /* the token list */
    int numOfTokens;
    int lineNum;
    boolean_t debug; /* can be set to TRUE or FLASE if wanted*/
    int errors; /* number of total erros in the tokenizer */
    boolean_t errFlag; /* flage that prevent from checking given word if a major spelling error was accoured (see getTokenType() in tokenizer.c )*/
} tokenizerState_t;

/* ----------------- tokenizer.c ------------------- */
extern tokenizerState_t *tokenizerState;

void initTokenizerState();
void clearTokenizerErrors();
boolean_t checkTokenizerErrors();

boolean_t checkValidLine(FILE *fp, char *str);
void discardInputLine(FILE *fp, char *str);

void lineToTokens(int lineNum, char *lineStr);
void clearTokenizerLine();
void printToken();

/* not found in string.h */
char *strdup(const char *s);

#endif /* _TOKEN_H_ */
