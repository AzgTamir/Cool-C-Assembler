#include <string.h>
#include <stdlib.h>
#include "token.h"
#include "data.h"
#include "stdio.h"
#include "parser.h"
#include "symTable.h"

static void parseReg(parserState *prsState, token_t *token);
static void parseOp(parserState *prsState, token_t *token);
static void parseLableDef(parserState *prsState, token_t *token);
static void parseLabelArg(parserState *prsState, token_t *token);
static void parseLabelAddr(parserState *prsState, token_t *token);
static void parseNumArg(parserState *prsState, token_t *token);
static void parseDataNum(parserState *prsState, token_t *token);
static void parseString(parserState *prsState, token_t *token);
void clearParserError();

static char *printAsmBin(bin_op_t *bop);

asm_cmd_t *asmCmd; 
int asmCmdSize;
/*General Explenation: Seconed process.

                       Frist Pass: the Parser gets array of tokens (see tokens in token.c ) 
                       and start writing thier binary vlaues into the 24 bit files (see bit fileds in data.h) 
                       while considering the structure and contex between each token. 
                       
                       Second Pass: the Parser gets over the array of bit fileds 
                                    and compliting missing infromation about labels  */

/*  the parserState represent the state of the pareser at any 
    given moment and holds important attributes about his current state
    (see more about the parserState in Praser.h)*/
parserState *prsState;

/* initialize parserState values */
void initParserState()
{
    static parserState prsStateInst;
    static boolean_t first=TRUE;
    if (first) { 
        asmCmd = NULL;
        first = FALSE;
    }

    asmCmdSize = MAX_ASM_CMD;
    asmCmd = realloc(asmCmd, asmCmdSize * sizeof(asm_cmd_t)); /* represent 1000 lines of binary code and their attributes*/
    memset(asmCmd, 0, asmCmdSize * sizeof(asm_cmd_t));

    prsState = &prsStateInst;
    prsState->_instCounter = 0;
    prsState->_opInstCounter = 0;
    prsState->_dataCounter = 0;
    prsState->_error = 0;


}

/**
 * @brief check for errors in parser 
 * 
 * @return boolean_t return if there was any error
 */
boolean_t checkPraserErrors()
{
    return prsState->_error != 0;
}

void clearParserError(){
    prsState->_error = 0;
}


/**
 * @brief free binary memory table 
 * 
 */
void destroyParserState() {
    free(asmCmd);
}
/**
 * @brief make sure we have enough memory (at least 100 more words) to place a new op
 * 
 */
void checkAsmCmdSize() {
    if(prsState->_instCounter + MAX_LINE_TOKENS > asmCmdSize ) {
        asm_cmd_t *tmp = realloc(asmCmd, (asmCmdSize + MAX_ASM_CMD)*sizeof(asm_cmd_t));
        if (tmp != NULL) {
            asmCmd = tmp;
            memset(asmCmd + asmCmdSize, 0, MAX_ASM_CMD * sizeof(asm_cmd_t));
            asmCmdSize += MAX_ASM_CMD;
            printf("Note: out of memory in line: %d, current AsmCmd size: %d, succesfully increased.\n", prsState->_line, asmCmdSize);
        } else {
            printf("Fatal: out of memory in line: %d, current AsmCmd size: %d \n", prsState->_line, asmCmdSize);
            exit(-1);
        }
    }
}


/**
 * @brief  first pass - start coding each "token" to his binary value 
               (coding "eampty" binary line to undefined labels) 
 * 
 * @param line line number
 * @param tokenList the given token list 
 * @param numOfTokens the num of tokens
 */
void parsePass1(int line, token_t *tokenList[], int numOfTokens)
{

    int numToken = 0;
    token_t *curToken;
    type_t tokenType;
    prsState->_line = line; /*uptading line number*/

    curToken = tokenList[numToken]; 

    prsState->_labelAttr = SYM_UNDEFINED; /*initialaize laebl attribute*/

    checkAsmCmdSize(); /*checks if theres enough space to code */

    for (numToken = 0; numToken < numOfTokens; numToken++, curToken++) /*running on token list*/
    {
        curToken = tokenList[numToken];
        tokenType = curToken->_type;

        switch (tokenType) /*checks for token type*/
        {
        case ENTRY_DIR:
            prsState->_labelAttr = SYM_ENTRY;
            break;
        case EXTERN_DIR:
            prsState->_labelAttr = SYM_EXTERN;
            break;
        case LABEL_DEF:
            prsState->_labelAttr = SYM_LOCAL;
            parseLableDef(prsState, curToken);
            /*no need to move instCounter's address, (just adding label to the label table ) */
            break;

        case OP:
            prsState->_opInstCounter = prsState->_instCounter;
            parseOp(prsState, curToken);
            prsState->_instCounter++; /* moving instCounter's address pointer, (writing to opInstCounter address ) */
            break;

        case REG_ARG:
            parseReg(prsState, curToken);
            prsState->_ArgPlace++; /* updating argument's place */
            break;

        case LABEL_ARG:
            parseLabelArg(prsState, curToken);
            if (prsState->_labelAttr != SYM_EXTERN && prsState->_labelAttr != SYM_ENTRY)
            {
                prsState->_instCounter++; /* moving opInstCounter's address pointer, (writing to instCounter address ) */
                prsState->_ArgPlace++;    /* updating argument's place */
            }
            break;

        case LABEL_ADDR:
            parseLabelAddr(prsState, curToken);
            prsState->_instCounter++; /* moving opInstCounter's address pointer, (writing to instCounter address ) */
            prsState->_ArgPlace++;    /* updating argument's place */
            break;

        case NUM_ARG:
            parseNumArg(prsState, curToken);
            prsState->_instCounter++; /* moving opInstCounter's address pointer, (writing to instCounter address ) */
            prsState->_ArgPlace++;    /* updating argument's place */
            break;

        case DATA_NUM:
            parseDataNum(prsState, curToken);
            prsState->_instCounter++; /* moving opInstCounter's address pointer, (writing to instCounter address ) */
            break;

        case STRING_STR:
            parseString(prsState, curToken);
            prsState->_instCounter++;
            break;
        default:
            break;
            /* ignoring Comma, New line  and undefined tokens */
        }
    }
}


/**
 * @brief gets an operand token and writes the binary value accordingly
 * 
 * @param prsState the parserState - to get current attributes
 * @param token the OP token
 */
void parseOp(parserState *prsState, token_t *token)
{
    int opNum = op_table[token->_subValue].opNum; 
    int funcNum = op_table[token->_subValue].funcNum;
    int opInstCounter = prsState->_opInstCounter;
    int argNum = op_table[token->_subValue].argsNum;

    asmCmd[opInstCounter]._binCmd._bin_op.op = opNum;
    asmCmd[opInstCounter]._binCmd._bin_op.func = funcNum;
    asmCmd[opInstCounter]._binCmd._bin_op.ARE = ABSOLUTE_ARE;
    asmCmd[opInstCounter]._type = OP;

    prsState->_ArgPlace = (argNum < 2) ? DST_ARG : SRC_ARG; /* updating arg's place according to the num of arguments  - see more in Parser.h */
}


/**
 * @brief gets an register token and writes the binary value accordingly 
            (while considering his own place in the operand code - whether it'll be source or destenation)
 * 
 * @param prsState the parserState - to get current attributes
 * @param token the REG type
 */
void parseReg(parserState *prsState, token_t *token)
{
    int opInstCounter = prsState->_opInstCounter;
    arg_place_t place = prsState->_ArgPlace;
    int regNum = token->_subValue;

    if (place == DST_ARG) 
    { /*destination*/
        asmCmd[opInstCounter]._binCmd._bin_op.dst_r = regNum;
        asmCmd[opInstCounter]._binCmd._bin_op.dst = ACC_REG;
    }
    else
    { /*source*/
        asmCmd[opInstCounter]._binCmd._bin_op.src_r = regNum;
        asmCmd[opInstCounter]._binCmd._bin_op.src = ACC_REG;
    }
}


/**
 * @brief gets label defenition type token and copy the label's string without the ":". 
 *        passes the infromation to the addLabelDef function.
 * 
 * @param prsState the parserState - to get current attributes
 * @param token the labekl deffinition token to be added
 */
void parseLableDef(parserState *prsState, token_t *token)
{
    int strLen = strlen(token->str);
    char str[SYM_NAME_LEN];
    strcpy(str, token->str);
    str[strLen - 1] = '\0'; /* writing 0 insted of ":" */

    addLabelDef(str, prsState->_instCounter, prsState->_labelAttr); /*add label to the label table (if needed) (symTable.c)*/
}


/**
 * @brief get label argument type and writes empty label values
 *        only if there was no decleration of any directive ( e.g: .entry or .extern ).
*           passes information to the addLabelArg  function.        
 * 
 * @param prsState the parserState - to get current attributes
 * @param token the label argument token to be added
 */
void parseLabelArg(parserState *prsState, token_t *token)
{
    int instCounter = prsState->_instCounter;
    int opInstCounter = prsState->_opInstCounter;
    arg_place_t place = prsState->_ArgPlace;
    addLabelArg(token->str, prsState->_labelAttr); /*add label to the label table (if needed) (symTable.c)*/

    if (prsState->_labelAttr != SYM_ENTRY && prsState->_labelAttr != SYM_EXTERN)
    {
        /* adds the label to the binary (still unknown values) */
        asmCmd[instCounter]._binCmd._bin_arg.value = 0;
        asmCmd[instCounter]._binCmd._bin_arg.ARE = UNDEFINED_ARE;
        asmCmd[instCounter]._type = token->_type;
        asmCmd[instCounter]._line = prsState->_line;
        strcpy(asmCmd[instCounter]._labelStr, token->str);

        if (place == DST_ARG)
        { /*destination*/
            asmCmd[opInstCounter]._binCmd._bin_op.dst = ACC_DIRECT;
        }
        else
        { /*source*/
            asmCmd[opInstCounter]._binCmd._bin_op.src = ACC_DIRECT;
        }
    }
}


/*passes the infromation to the addLabelArgAddr function*/
/**
 * @brief get label argument type and writes empty label values.
 *        sendes infromation to the addLabelArgAddr.
 * 
 * @param prsState the parserState - to get current attributes
 * @param token the label address-type token to be added
 */
void parseLabelAddr(parserState *prsState, token_t *token)
{
    int instCounter = prsState->_instCounter;
    int opInstCounter = prsState->_opInstCounter;
    arg_place_t place = prsState->_ArgPlace;

    addLabelArgAddr(token->str + 1, prsState->_labelAttr); /*add label to the label table (if needed) (symTable.c)*/

    /* adds the label to the binary (still unknown values) */
    asmCmd[instCounter]._binCmd._bin_arg.value = 0;
    asmCmd[instCounter]._binCmd._bin_arg.ARE = UNDEFINED_ARE;
    asmCmd[instCounter]._type = token->_type;
    asmCmd[instCounter]._line = prsState->_line;
    strcpy(asmCmd[instCounter]._labelStr, token->str);
    if (place == DST_ARG)
    { /* destination*/
        asmCmd[opInstCounter]._binCmd._bin_op.dst = ACC_RLTV;
    }
    else
    { /*source*/
        asmCmd[opInstCounter]._binCmd._bin_op.src = ACC_RLTV;
    }
}


/**
 * @brief gets num argument token type and write his value accordingly (ARE included)
 * 
 * @param prsState the parserState - to get current attributes
 * @param token the num argument-type (e.g: #4 )
 */
void parseNumArg(parserState *prsState, token_t *token)
{
    int instCounter = prsState->_instCounter;
    asmCmd[instCounter]._binCmd._bin_arg.value = token->_subValue;  /*writing value */
    asmCmd[instCounter]._binCmd._bin_arg.ARE = ABSOLUTE_ARE;
}


/**
 * @brief gets num data token type and write his value accordingly (full 24 bits)
 * 
 * @param prsState the parserState - to get current attributes
 * @param token the num data-type (e.g: 5 ) 
 */
void parseDataNum(parserState *prsState, token_t *token)
{
    int instCounter = prsState->_instCounter;
    asmCmd[instCounter]._binCmd._bin_dir.value = token->_subValue; /*writing value */
    prsState->_dataCounter++;
}


/**
 * @brief gets string type token and write each character's value in the string (full 24 bits to each char) 
 * 
 * @param prsState the parserState - to get current attributes
 * @param token the string token-Type (e.g "hello")
 */
void parseString(parserState *prsState, token_t *token)
{
    int instCounter = prsState->_instCounter;
    int i = 1; /* to skip the " in the beggining of the string */
    char *str = token->str;
    for (; i < strlen(str) - 1; i++, instCounter++)
    { /* i < strlen(str)-1 to skip the " in the end of the string */
        asmCmd[instCounter]._binCmd._bin_dir.value = str[i];
        prsState->_dataCounter++;
    }
    asmCmd[instCounter]._binCmd._bin_dir.value = '\0'; /*makes char value null terminated*/
    prsState->_dataCounter++;
    prsState->_instCounter = instCounter;
}


/**
 * @brief Second pass - compliting missing coding infromation and value to undefined labels
 * 
 * @return int number of errors that were occurred 
 */
int parsePass2()
{
    int i, index, numOfCmd = prsState->_instCounter;
    int errNum = 0;

    for (i = 0; i < numOfCmd; i++)
    { /* scaning all commands */

        if (asmCmd[i]._binCmd._bin_arg.value == 0)
        { /* looking for empty value command */

            if (asmCmd[i]._type == LABEL_ARG)
            {
                if ((index = getLabel(asmCmd[i]._labelStr)) != -1 && labelTable[index].LabelAttr != SYM_UNDEFINED)
                {
                    asmCmd[i]._binCmd._bin_arg.value = labelTable[index].addr; /*token type is label argument - 
                                                                                write the original label defenition address to the value fild*/
                    if (labelTable[index].LabelAttr == SYM_EXTERN)
                    { /*writing the ARE attribute accordining to the original label defenition*/
                        asmCmd[i]._binCmd._bin_arg.ARE = EXTERNAL_ARE;
                    }
                    else
                    {
                        asmCmd[i]._binCmd._bin_arg.ARE = RELOCAL_ARE;
                    }
                }
                else
                {
                    printf("\033[1;31mWarning:\033[0m Line: %d => label has no definition: '%s' \n" , asmCmd[i]._line, labelTable[index].symStr);
                    errNum++;
                }
            }
            else if (asmCmd[i]._type == LABEL_ADDR)
            {
                if ((index = getLabel(asmCmd[i]._labelStr + 1)) != -1  && labelTable[index].LabelAttr != SYM_UNDEFINED)
                { /*token type is lebal address - write the address relative to the original defenation label address */
                    if (labelTable[index].LabelAttr != SYM_EXTERN)
                    {
                        asmCmd[i]._binCmd._bin_arg.value = labelTable[index].addr - ((i - 1) + BASE_INST_COUNTER);
                        asmCmd[i]._binCmd._bin_arg.ARE = ABSOLUTE_ARE;
                    }
                    else
                    {
                        asmCmd[i]._binCmd._bin_arg.value = 0; /* cannot calculate offset to extern label */
                        asmCmd[i]._binCmd._bin_arg.ARE = EXTERNAL_ARE;
                    }
                }
                else
                {
                    printf("\033[1;31mWarning:\033[0m Line: %d => Label address has no definition: '%s' \n", prsState->_line, labelTable[index].symStr );
                    errNum++;
                }
            }
        }
    }
    return errNum;
}

/**
 * @brief printing the hole file code after prosses 
 * 
 */
void parserAsmCmdPrint()
{
    int i = 0;
    int instCounter = prsState->_instCounter;
    int numOfData = prsState->_dataCounter;

    printf("\n code size: %d,  data size: %d  \n", instCounter - numOfData, numOfData);

    for (; i < prsState->_instCounter; i++)
    {
        printf("Address: %07d  Cmd: %02x%02x%02x  -- %s\n", i + BASE_INST_COUNTER,
               asmCmd[i]._binCmd._bin_cmd[2],
               asmCmd[i]._binCmd._bin_cmd[1],
               asmCmd[i]._binCmd._bin_cmd[0],
               asmCmd[i]._type == OP ? printAsmBin(&asmCmd[i]._binCmd._bin_op) : asmCmd[i]._labelStr);
    }
}

/**
 * @brief printing operand attributes
 * 
 * @param binOp the binary code of given operand 
 * @return char* the dtring of every operand attribute 
 */
char *printAsmBin(bin_op_t *binOp)
{
    static char scmd[MAX_ASM_PRINT_LEN];
    sprintf(scmd, "op:%0x src:%d src_r:%d dst:%d dst_r:%d func:%0x, ARE:%d", binOp->op, binOp->src, binOp->src_r, binOp->dst, binOp->dst_r, binOp->func, binOp->ARE);
    return scmd;
}