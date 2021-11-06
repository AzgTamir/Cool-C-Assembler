#ifndef _SYNTAX_CHECKER_H_
#define _SYNTAX_CHECKER_H_

#include "data_defs.h"
#include "token.h"
#include "data.h"

typedef enum
{
    INVALID_ARG = 0,
    VALID_ARG = 1
} valid_arg_t;

/* ------------ syntaxChecker.c -------------*/
boolean_t syntaxChecker(int line, token_t *cmdLine[], int numOfTokens);

#endif /* _SYNTAX_CHECKER_H_ */