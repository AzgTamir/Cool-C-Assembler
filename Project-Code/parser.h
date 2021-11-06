#ifndef PARSER_H
#define PARSER_H

#include "data.h"

typedef enum
{
    NO_ARG = 0,
    SRC_ARG = 1, /* srouce */
    DST_ARG = 2 /* destanation */
} arg_place_t; /* DOES NOT correlate with numOfArgs -- repr. argument's state (whether the argument should set source or destenation) */

typedef struct parserState
{
    int _instCounter;   /* instraction counter a.k.a IC */
    int _opInstCounter; /* operand instraction counter */

    arg_place_t _ArgPlace; /* simbolize the current argument state (whether the argument it's source or destenation) */

    int _line;
    sym_attr_t _labelAttr; /* simbolize the current label attribute e.g: external/entry/local/undefined */
    int _dataCounter;      /* number of data a.k.a DC */

    int _error;
} parserState;

extern parserState *prsState;

/* --------- Parser ------------- */
void initParserState();
void destroyParserState();
boolean_t checkPraserErrors();
void clearParserError();

void parsePass1(int line, token_t *tokenList[], int numOfTokens);
int parsePass2();

void parserAsmCmdPrint();

#endif /* PARSER_H  */