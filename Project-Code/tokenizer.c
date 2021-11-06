#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "token.h"
#include "data.h"
#include "parser.h"

static int splitStr(char *str, char *wordArr[]);
static void wordsToToken(int lineNum, char *line[], int nwords, token_t *arrToken[]);

static token_t *newToken();
static void delToken(token_t *tok);
static void setAccMod(token_t *token);

static type_t getTokenType(char *, int *, int line);
static char *printType(type_t);

static int checkOp(char *, int *);
static int checkLabel(char *, int line);
static int checkReg(char *, int *);
static int checkDot(char *, int line);
static int checkNum(char *, int *, int line);

/*General Explenation: First Process. 
                       The Tokenizer gets a sentence of commaned, 
                       seperate them into individual words (while keeping importent charcters e.g: "," ). 
                       Then he analize each word and "tag" them accordingly
                       to thier meaning (such as operands, directives, labels and more see options of diffrent meanings in Token.h) 
                       after the "taging" process has done the tokenizer sendes the array of token to the Praser.(see Parser in Parser.c) */

/* the table that contains info on every operand information */
const cmd_ref_table op_table[OP_TABLE_NUM] = {
    {"mov", OP_MOV, 0, 2,
     /*srcArg*/ {LABEL_ARG, REG_ARG, NUM_ARG, NUL_TOK},
     /*dstArg*/ {LABEL_ARG, REG_ARG, NUL_TOK}},
    {"cmp", OP_CMP, 0, 2,
     /*srcArg*/ {LABEL_ARG, REG_ARG, NUM_ARG, NUL_TOK},
     /*dstArg*/ {LABEL_ARG, REG_ARG, NUM_ARG, NUL_TOK}},
    {"add", OP_ADD_SUB, 1, 2,
     /*srcArg*/ {LABEL_ARG, REG_ARG, NUM_ARG, NUL_TOK},
     /*dstArg*/ {LABEL_ARG, REG_ARG, NUL_TOK}},
    {"sub", OP_ADD_SUB, 2, 2,
     /*srcArg*/ {LABEL_ARG, REG_ARG, NUM_ARG, NUL_TOK},
     /*dstArg*/ {LABEL_ARG, REG_ARG, NUL_TOK}},
    {"lea", OP_LEA, 0, 2,
     /*srcArg*/ {LABEL_ARG, NUL_TOK},
     /*dstArg*/ {LABEL_ARG, REG_ARG, NUL_TOK}},
    {"clr", OP_CLR_NOT_INC_DEC, 1, 1,
     /*srcArg*/ {NUL_TOK},
     /*dstArg*/ {LABEL_ARG, REG_ARG, NUM_ARG, NUL_TOK}},
    {"not", OP_CLR_NOT_INC_DEC, 2, 1,
     /*srcArg*/ {NUL_TOK},
     /*dstArg*/ {LABEL_ARG, REG_ARG, NUL_TOK}},
    {"inc", OP_CLR_NOT_INC_DEC, 3, 1,
     /*srcArg*/ {NUL_TOK},
     /*dstArg*/ {LABEL_ARG, REG_ARG, NUL_TOK}},
    {"dec", OP_CLR_NOT_INC_DEC, 4, 1,
     /*srcArg*/ {NUL_TOK},
     /*dstArg*/ {LABEL_ARG, REG_ARG, NUL_TOK}},
    {"jmp", OP_JMP_BNE_JSR, 1, 1,
     /*srcArg*/ {NUL_TOK},
     /*dstArg*/ {LABEL_ARG, LABEL_ADDR, NUL_TOK}},
    {"bne", OP_JMP_BNE_JSR, 2, 1,
     /*srcArg*/ {NUL_TOK},
     /*dstArg*/ {LABEL_ARG, LABEL_ADDR, NUL_TOK}},
    {"jsr", OP_JMP_BNE_JSR, 3, 1,
     /*srcArg*/ {NUL_TOK},
     /*dstArg*/ {LABEL_ARG, LABEL_ADDR, NUL_TOK}},
    {"red", OP_RED, 0, 1,
     /*srcArg*/ {NUL_TOK},
     /*dstArg*/ {LABEL_ARG, REG_ARG, NUL_TOK}},
    {"prn", OP_PRN, 0, 1,
     /*srcArg*/ {NUL_TOK},
     /*dstArg*/ {NUM_ARG, LABEL_ARG, REG_ARG, NUL_TOK}},
    {"rts", OP_RTS, 0, 0,
     /*srcArg*/ {NUL_TOK},
     /*dstArg*/ {NUL_TOK}},
    {"stop", OP_STOP, 0, 0,
     /*srcArg*/ {NUL_TOK},
     /*dstArg*/ {NUL_TOK}},
    {"NULL", OP_NULL, -1, -1, /*srcArg*/ {NUL_TOK}, /*dstArg*/ {NUL_TOK}}};

tokenizerState_t *tokenizerState;


/**
 * @brief initilaize tokenizerState's attributes (see more about tokenizerState in token.h )
 * 
 */
void initTokenizerState()
{
    static tokenizerState_t instTokenizerState;
    tokenizerState = &instTokenizerState;

    tokenizerState->numOfTokens = 0;
    tokenizerState->lineNum = 0;
    tokenizerState->debug = TRUE; 
    tokenizerState->errors = 0;
    tokenizerState->errFlag = FALSE;
}

/**
 * @brief clearing tokenizer error counter
 * 
 */
void clearTokenizerErrors()
{
    tokenizerState->errors = 0;
}

/**
 * @brief check if there is no errors in the tokenzier
 * 
 * @return boolean_t if there is no errors in the tokenzier
 */
boolean_t checkTokenizerErrors()
{
    return tokenizerState->errors != 0;
}


/**
 * @brief source input support functions for tokenizer (setting \ n at end of line)
 * 
 * @param fp given file
 * @param fileName filename
 * @return boolean_t return if succeed
 * 
 */
boolean_t checkValidLine(FILE *fp, char *fileName)
{
    int strLen = strlen(fileName);
    if (fileName[strLen - 1] == '\n')
        return TRUE;
    if (feof(fp))
    { /* legit corner case for missing EOL at EOF */
        fileName[strLen] = '\n';
        fileName[strLen + 1] = '\0';
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief discarding excess characters in line (if more then 80)
 * 
 * @param fp the given file
 * @param str the given char pointer 
 */
void discardInputLine(FILE *fp, char *str)
{
    int strLen;
    for (strLen = strlen(str); !feof(fp) && str[strLen - 1] != '\n'; strLen = strlen(str))
    {
        fgets(str, MAX_LINE_LEN, fp);
    }
}

/* ------- main function of the tokenizer ------- */
/**
 * @brief main function of the tokenize
 * 
 * @param lineNum current line num
 * @param lineStr the given line string
 */
void lineToTokens(int lineNum, char *lineStr)
{
    tokenizerState->lineNum = lineNum;
    tokenizerState->numOfTokens = splitStr(lineStr, tokenizerState->lineWords);                               /* spliting the given sentence to seperate words */
    wordsToToken(lineNum, tokenizerState->lineWords, tokenizerState->numOfTokens, tokenizerState->tokenList); /* tag each word according to the meaning of it*/
    if (tokenizerState->debug)
        printToken();
}

/**
 * @brief free tokenzer string list and token list
 * 
 */
void clearTokenizerLine()
{
    int i;
    for (i = 0; i < tokenizerState->numOfTokens; i++)
    { /* free the array of words and the array of tokens */
        free(tokenizerState->lineWords[i]);
        delToken(tokenizerState->tokenList[i]);
    }
    clearTokenizerErrors();
}


/**
 * @brief coping one word to the another with given offset using malloc 
 * 
 * @param firstPtr first pointer
 * @param lastPtr second pointer
 * @return char* char pointer to the new word
 */
char *Mystrndup(char *firstPtr, char *lastPtr)
{
    int len = lastPtr - firstPtr;
    char *tmpPtr = malloc(len + 1);
    strncpy(tmpPtr, firstPtr, len);
    tmpPtr[len] = '\0';
    return tmpPtr;
}

/**
 * gets command line; spliting the given sentence to seperate words 
 * @param str : input str
 * @param wordArr : output arry of words
 **/
int splitStr(char *str, char *wordArr[])
{
    int index = 0;
    char *firstPtr = str;
    char *lastPtr = firstPtr;

    while (strchr(" \t", *firstPtr)) /*ignoring space in start of line*/
    {
        firstPtr++;
    }
    if (strchr(";\n", *firstPtr)) /*returns if comment or \ n spotted in beginning of line*/ 
        return 0;
    while (*firstPtr)
    {
        /*both pointers at beginning of word -  moving lastPointer until end of word */
        for (lastPtr = firstPtr; *lastPtr && !strchr(" ,;\"\t\n", *lastPtr); lastPtr++); 
        
        /* extracting word */
        if (*lastPtr && (lastPtr != firstPtr))
        {
            wordArr[index++] = Mystrndup(firstPtr, lastPtr);
        }
        if (*lastPtr == ',') /*if lastPointer pointing on ',' - then its added */
            wordArr[index++] = strdup(",");
        if (strchr(";\n", *lastPtr)) /*if lastPointer pointing on comment or new line then return*/
            break;
        if (*lastPtr == '"') /* corner case: if \" was spotted then extrating sentece/word until closure */
        {
            firstPtr = lastPtr;
            lastPtr = strchr(firstPtr + 1, '"');
            if (lastPtr != NULL)
            {
                wordArr[index++] = Mystrndup(firstPtr, lastPtr + 1);
            }
            else
            {
                printf("\n\033[1;31mWarning:\033[0m Line: %d => missing \" \n", tokenizerState->lineNum);
                tokenizerState->errors++;
                break;
            }
        }
        firstPtr = lastPtr + 1;
    }
    return index;
}


/**
 * @brief gets token pointer and free the needed allocated memory
 * 
 * @param tok token to be free
 */
void delToken(token_t *tok)
{
    free(tok->str);
    free(tok);
}


/**
 * @brief token's constractor
 * 
 * @return token_t* allocated token
 */
token_t *newToken()
{
    token_t *ptr = (token_t *)malloc(sizeof(token_t));
    ptr->_subValue = 0;
    ptr->str = NULL;
    ptr->_type = UNDEFINED_TOKEN;
    return ptr;
}

/*gets an array of words and tag each word according to the meaning of it*/
/**
 * @brief convert words to tokens
 * 
 * @param lineNum - input line number
 * @param line - input line words
 * @param nwords - number of words
 * @param arrToken - output array of tokens
 */
void wordsToToken(int lineNum, char *line[], int nwords, token_t *arrToken[])
{
    int i = 0;
    int tokenArrPtr = 0;
    int enumValue, subValue;

    for (i = 0; i < nwords; i++)
    {
        arrToken[tokenArrPtr] = newToken(); /*allocating token*/

        enumValue = getTokenType(line[i], &subValue, lineNum); /*getting token type*/
        if (enumValue != UNDEFINED_TOKEN)
        {   
            /*adding token to the token list*/
            arrToken[tokenArrPtr]->str = strdup(line[i]);
            arrToken[tokenArrPtr]->_type = enumValue;
            arrToken[tokenArrPtr]->_subValue = subValue;
            setAccMod(arrToken[tokenArrPtr]);
        }
        else
        {
            /*undefined token => still adding but reporting problem*/
            tokenizerState->errors++;
            arrToken[tokenArrPtr]->str = strdup(line[i]);
            arrToken[tokenArrPtr]->_type = enumValue;

            printf("\n\033[1;31mWarning:\033[0m Line: %d => undefined token: '%s'  \n", lineNum,line[i]);
        }
        tokenArrPtr++;
    }
}

/**
 * @brief gets token and sets the access mode according to it's type
 * 
 * @param token token to be checked
 */
void setAccMod(token_t *token)
{
    switch (token->_type)
    {
    case REG_ARG:
        token->_accMod = ACC_REG;
        break;
    case NUM_ARG:
        token->_accMod = ACC_IMMDT;
        break;
    case LABEL_ARG:
        token->_accMod = ACC_DIRECT;
        break;
    case LABEL_ADDR:
        token->_accMod = ACC_RLTV;
        break;
    default:
        break;
    }
}



/**
 * @brief gets a string and return his type - acordding to the analized string
 * 
 * @param str toke's string 
 * @param subValue used to determine subs values of certian type. e.g r1 --> subValue = 1; #-67 --> subType = -67
 * @param line current line
 * @return type_t the currect token type
 */
type_t getTokenType(char *str, int *subValue, int line)
{
    int value;
    int strLen = strlen(str);
    tokenizerState->errFlag = FALSE;

    value = checkOp(str, subValue);
    if (value == OP)
    {
        return value;
    }

    value = checkReg(str, subValue);
    if (value == REG_ARG)
    {
        return value;
    }

    if (*str == ',')
    {
        return COMMA;
    }

    if (*str == '\n')
    {
        return LINE;
    }

    value = checkNum(str, subValue, line);
    if (value == DATA_NUM || value == NUM_ARG)
    {
        return value;
    }
    if (tokenizerState->errFlag) return UNDEFINED_TOKEN;

    value = checkDot(str, line);
    if (value == DATA_DIR || value == STRING_DIR || value == EXTERN_DIR || value == ENTRY_DIR)
    {
        return value;
    }
    if (tokenizerState->errFlag) return UNDEFINED_TOKEN;


    if (str[0] == '"' && str[strLen - 1] == '"')
    {
        return STRING_STR;
    }

    value = checkLabel(str, line);
    if (value == LABEL_DEF || value == LABEL_ADDR || value == LABEL_ARG)
    {
        return value;
    }

    return UNDEFINED_TOKEN;
}


/**
 * @brief gets a string and return whether its an operand type token
 * 
 * @param opToCheck the string to be checked
 * @param subValue stores the OP num (MOV = 0)
 * @return  whether its OP-type token
 */
int checkOp(char *opToCheck, int *subValue)
{
    int i = 0;
    for (; i < 17; i++)
    {
        if (!strcmp(opToCheck, op_table[i].str))
        { /* using the op table */
            *subValue = i;
            return OP;
        }
    }
    return UNDEF_OP;
}

/**
 * @brief gets a string and return whether its an label-argument type token
 * 
 * @param labelToCheck the label to be checked
 * @param line current line number
 * @return whether its Labrl-argument-type token 
 */
int checkLabel(char *labelToCheck, int line)
{
    int labelLen = strlen(labelToCheck);
    int labelStart = 0, labelEnd=labelLen;
    char *cleanLabel;

    type_t labelType = LABEL_ARG;
    int subValue=0;

    if ('.' == labelToCheck[0])
    { /*checking for something like ".data:" --> illegal label*/
        printf("\n\033[1;31mWarning:\033[0m line: %d  => label cannot be dirrctive-like, label: %s \n", line, labelToCheck);
        tokenizerState->errFlag=TRUE;
        return UNDEF_LAB;
    }


    /*determine between diffrent kinds of labels argument OR address; e.g: hello: OR &hello */
    if (':' == labelToCheck[labelLen - 1])
    {
        labelType=LABEL_DEF;
        labelEnd--; /* cutting from the end,  because of ":" */
        labelLen--;
    }
    else if ('&' == labelToCheck[0])
    {
        labelType=LABEL_ADDR;
        labelStart++; /*cutting from start, because of "&" */
        labelLen--;
    }
    
    if (isalpha(labelToCheck[labelStart])) /*making sure label starts with a letter */
    {
        int i;
        for (i = labelStart+1; i <= labelEnd; i++)
            if (!isalnum(labelToCheck[i]) ) /*making sure label contain only letters and digit*/
                break;
        if (i < labelEnd) { 
            printf("\n\033[1;31mWarning:\033[0m line: %d => label must have letters and digit chars only, label: '%s' \n", line, labelToCheck);
            labelType = UNDEF_LAB;
            tokenizerState->errFlag=TRUE;
        }
    } else { 
        labelType = UNDEF_LAB;
        printf("\n\033[1;31mWarning:\033[0m line: %d => label must begin with a letter, label: '%s' \n", line, labelToCheck);
        tokenizerState->errFlag=TRUE;
    }

    
    cleanLabel = Mystrndup(labelToCheck+labelStart, labelToCheck+labelEnd); /* getting "clean" label (without & or : )*/
    if (checkReg(cleanLabel, &subValue) == REG_ARG)
    { /*checking for something like "r3:" --> illegal label*/
        printf("\n\033[1;31mWarning:\033[0m line: %d  => label cannot be register-like, label: %s \n", line, labelToCheck);
        labelType= UNDEF_LAB;
        tokenizerState->errFlag=TRUE;
    }
    if (checkOp(cleanLabel, &subValue) == OP)
    { /*checking for something like "mov:" --> illegal label*/
        printf("\n\033[1;31mWarning:\033[0m line: %d  =>label cannot be Op-like, label: %s \n", line, labelToCheck);
        labelType= UNDEF_LAB;
        tokenizerState->errFlag=TRUE;
    }
    free(cleanLabel);

    if (labelLen > 31) { /*making sure label is in right length*/
        labelType=UNDEF_LAB;
        printf("\033[1;31mWarning:\033[0m Line: %d => too long of a label, label: %s \n",line ,labelToCheck);
        tokenizerState->errFlag=TRUE;
    }

    return labelType;
}

/**
 * @brief gets a string and return whether its a register or not - acordding to the analized string
 * 
 * @param regToCheck register to check
 * @param subValue contain the number of register e.g: r3 => subValue = 3
 * @return int whether the give string is a register
 */
int checkReg(char *regToCheck, int *subValue)
{
    int regLen = strlen(regToCheck);

    if (regToCheck[0] != 'r' || !isdigit(regToCheck[1]))
        return UNDEF_REG;

    if (regLen == 2 && regToCheck[1] >= '0' && regToCheck[1] <= '7')
    {
        *subValue = regToCheck[1] - '0';
        return REG_ARG;
    }

    return UNDEF_REG;
}

/**
 * @brief gets a string and return whether its some kind of directive - acordding to the analized string
 * 
 * @param str the string to be checked
 * @param line the current line number
 * @return int whether the string is a directive 
 */
int checkDot(char *str, int line)
{
    int tmp = 0;
    if (str[0] == '.')
    {
        if(checkOp(str+1,&tmp) == OP){
            printf("\n\033[1;31mWarning:\033[0m Line: %d => dirctive cannot be opernad-like: %s", tokenizerState->lineNum ,str);
            tokenizerState->errFlag=TRUE;
            return UNDEFINED_TOKEN;
        }
        if (!strcmp(str, ".data"))
        {
            return DATA_DIR;
        }
        else if (!strcmp(str, ".string"))
        {
            return STRING_DIR;
        }
        else if (!strcmp(str, ".extern"))
        {
            return EXTERN_DIR;
        }
        else if (!strcmp(str, ".entry"))
        {
            return ENTRY_DIR;
        }
        printf("\n\033[1;31mWarning:\033[0m line: %d  => illegal directive, dir: %s,   \n", line, str); /* looking for something like: .hello --> illegal */
        tokenizerState->errFlag=TRUE;
    }
    return UNDEF_DOT;
}

/**
 * @brief gets a string and return whether its some kind of number - immediate: #4 OR data: 4,5,6 
 * 
 * @param str the string to be checked
 * @param subValue contain the number (e.g: #67 => subValue = 67)
 * @param line the current line number
 * @return int 
 */
int checkNum(char *str, int *subValue, int line)
{
    int strLen = strlen(str);
    type_t numType = UNDEF_NUM;
    int i;

    if (str[0] == '#') /* if immediate */
    {
        numType = NUM_ARG;
        if (str[1] == '-' || str[1] == '+')
        {
            i = 2; /*start checking after #- OR #+ */
        }
        else
        {
            i = 1; /* start checking after #  */
        }
    }
    else if ((str[0] >= '0' && str[0] <= '9') || str[0] == '-' || str[0] == '+') /*if data number*/
    {
        numType = DATA_NUM;
        if (str[0] == '-' || str[0] == '+')
        {
            i = 1; /* start checking after - OR +  */
        }
        else
        {
            i = 0; /* start checking from the first number  */
        }
    }
    else
    {
        return UNDEF_NUM;
    }

    for (; i < strLen; i++)
    {
        if (!(str[i] >= '0' && str[i] <= '9'))
        { /* checking valid number something like: #45hb5 OR 57h84 OR 3.45 --> illegal number */

            printf("\n\033[1;31mWarning:\033[0m line: %d  => illegal number, number: %s  \n", line, str);
            tokenizerState->errFlag=TRUE;
            return UNDEF_NUM;
            

        }
    }

    if (numType == NUM_ARG)
    { /*num type is argument. e.g: #4*/
        const int max_int21 = (int)(0x000fffff); /*  2^20 - 1 */
        const int min_int21 = (int)(0xfff00000); /* -2^20     */

        *subValue = atoi(str + 1);
        if (*subValue > max_int21 || *subValue < min_int21)
        { /*lowest & highest 21 bits number */
            printf("\n\033[1;31mWarning:\033[0m line: %d  => immediate number overflow, number: %d \n", line, *subValue);
            tokenizerState->errFlag=TRUE;
            return UNDEF_NUM;
        }
    }
    else /*num type is data. e.g: 5 */
    {
        const int max_int24 = (int)(0x007fffff); /*  2^23 - 1 */
        const int min_int24 = (int)(0xff800000); /* -2^23     */

        *subValue = atoi(str);
        if (*subValue > max_int24 || *subValue < min_int24)
        { /*lowest & highest 24 bits number */
            printf("\n\033[1;31mWarning:\033[0m line: %d  => data number overflow, number: %d \n", line, *subValue);
            tokenizerState->errFlag=TRUE;
            return UNDEF_NUM;
        }
    }

    return numType;
}

/**
 * @brief prits token-type attributes
 * 
 */
void printToken()
{
    int i = 0;
    while (i < tokenizerState->numOfTokens)
    {
        printf("Type: %s : '%s' \n", printType(tokenizerState->tokenList[i]->_type), tokenizerState->tokenList[i]->str);
        i++;
    }
}


/**
 * @brief gets type of token and return his string
 * 
 * @param type the given type
 * @return char* the token type's string 
 */
char *printType(type_t type)
{
    static char *tokNames[] = { 
        "UNDEFINED_TOKEN", "OP", "LABEL_DEF", "LABEL_ADDR", "LABEL_ARG", "REG_ARG", "NUM_ARG", "DATA_DIR", 
        "DATA_NUM", "STRING_DIR", "STRING_STR", "ENTRY_DIR", "EXTERN_DIR", "LINE", "COMMA" 
    };
    return tokNames[type > 0 ? type : 0];
}