#include <stdio.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "data.h"
#include "parser.h"
#include "token.h"
#include "syntaxChecker.h"

boolean_t thereIsALabel;

static valid_arg_t checkValidArg(const cmd_ref_table *retCmdRef, token_t *argTok, arg_place_t argPlace, boolean_t secondArg);
static boolean_t opRoute(token_t *cmdLine[], int arraylength, int index);
static boolean_t dirRoute(token_t *cmdLine[], int arraylength, int index);

static boolean_t checkNumArgsDir(type_t dirType, int index, int arrayLength, int line);
static boolean_t checkTypeArgsDir(type_t dirType, type_t argType, int line, token_t *argTok);

static const cmd_ref_table *findCmdRef(token_t command);

/* General explantion: the syntaxChecker part ob is to check the validity tokens arrengment of given instruction line 
main function: syntaxChecker 
syntaxChecker: checks for LABEL existence and calling dirRoute or opRoute
dirRoute: checks for validity in all kinds of instructionfrom format .something
opRoute: in charge of sending each token line to her right group, he checks for making sure that the line doesnt have any unwanted tokens at her end
group1checkup: check syntac for mov, add, sub, cmp, lea. all of the 2 arguments instruction
 */

/* ------ main function of the syntax checker ------- */
/**
 * @brief the function recieve the instruction devided to tokens' the line number and amount of tokens at the line
 * 
 * @param lineNum - int
 * @param cmdLine - array of token pointers, each pointer is pointing to a wird in the instruction
 * @param numOfTokens - int, cmdLIne length
 * @return boolean_t  - TRUE or FALSE
 */
boolean_t syntaxChecker(int lineNum, token_t *cmdLine[], int numOfTokens)
{
	int index = 0;
	if (cmdLine[0]->_type == LABEL_DEF)
	{
		index++;
	}
	if (index == numOfTokens)
		return TRUE; /* situation were the line held only a LABEL defention. */
	else
	{
		if (cmdLine[index]->_type != OP)/* either instruction line or data line*/
			return dirRoute(cmdLine, numOfTokens, index);
		else
			return opRoute(cmdLine, numOfTokens, index);
	}
}



/**
 * @brief checking if reached end of token line
 * 
 * @param tokArg - array of token pointers, each pointer is pointing to a wird in the instruction
 * @param index - int
 * @param arrayLength - int 
 * @return boolean_t - TRUE or FALSE
 */
boolean_t checkNoArgs(token_t *tokArg[], int index, int arrayLength)
{
	int isComma = 1;
	if (index == arrayLength)/* index reached end of line*/
		return TRUE;
	for(; index < arrayLength; index++){
		if(tokArg[index]->_type != COMMA){
			isComma = 0;
		}
	}
	if (!isComma)
		printf("\n\033[1;31mWarning:\033[0m Line: %d, => too many args \n", tokenizerState->lineNum);
	else
		printf("\n\033[1;31mWarning:\033[0m Line: %d, => invalid comma at end of line \n", tokenizerState->lineNum);
	return FALSE;
}

/**
 * @brief checking if there are enough args
 * 
 * @param index - int
 * @param arrayLength - int
 * @return boolean_t - TRUE or FALSE
 */
boolean_t checkMoreArgs(int index, int arrayLength)
{
	if (index < arrayLength)
		return TRUE;
	printf("\033[1;31mWarning:\033[0m Line: %d, => not enough args \n", tokenizerState->lineNum);
	return FALSE;
}
/**
 * @brief checking if the commas are placed correctly
 * 
 * @param argTok - token pointer, pointer to a token that represent a word
 * @param secondArg - TRUE or FALSE
 * @return boolean_t - TRUE or FALSE
 */
boolean_t printInvalidCommaArg(token_t *argTok, boolean_t secondArg)
{
	if (secondArg && argTok->_type == COMMA)
		printf("\033[1;31mWarning:\033[0m Line: %d, => too many consecutive comma's \n", tokenizerState->lineNum );
	else
		printf("\033[1;31mWarning:\033[0m Line: %d, => invalid %s: '%s'  \n", tokenizerState->lineNum, argTok->_type == COMMA ? "comma" : "arg",argTok->str );
	return FALSE;
}

/*############# all of those function return a boolean_t value for if there is an eror ###############*/
/**
 * @brief main function to check the syntax of opration instruction
 * 
 * @param cmdLine -> array of token pointers, represent the instruction line divided to tokens
 * @param arraylength - int
 * @param index - int
 * @return boolean_t - TRUE or FALSE, found erors - false else true
 */
boolean_t opRoute(token_t *cmdLine[], int arraylength, int index)
{
	const cmd_ref_table *retCmdRef = findCmdRef(*cmdLine[index++]);

	if (retCmdRef->funcNum == OP_NULL)
	{
		printf("\n\033[1;31mWarning:\033[0m line: %d => Undefined instruction.\n", tokenizerState->lineNum);
		return FALSE;
	}

	if (retCmdRef->argsNum == 0)
	{
		if (checkNoArgs(cmdLine, index, arraylength))
			return TRUE;
		return FALSE;
	}
	if (!checkMoreArgs(index, arraylength))
		return FALSE;
	if (checkValidArg(retCmdRef, cmdLine[index++], retCmdRef->argsNum == 1 ? DST_ARG : SRC_ARG, FALSE) != VALID_ARG)
		return FALSE;

	if (retCmdRef->argsNum == 2)
	{
		if (!checkMoreArgs(index, arraylength))
			return FALSE;
		if (cmdLine[index++]->_type != COMMA)
		{
			printf("\033[1;31mWarning:\033[0m Line: %d, => missing comma \n", tokenizerState->lineNum);
			return FALSE;
		}

		if (!checkMoreArgs(index, arraylength))
			return FALSE;
		if (checkValidArg(retCmdRef, cmdLine[index++], DST_ARG, TRUE) != VALID_ARG)
			return FALSE;
	}

	if (!checkNoArgs(cmdLine, index, arraylength))
		return FALSE;

	return TRUE;
}
/**
 * @brief checking if the arg isvalid
 * 
 * @param retCmdRef  - cmd_ref_table a struct that contain the instruction information
 * @param argTok - pointer to token, represent a word in the instruction
 * @param argPlace - if we are looking for the src or dest argument
 * @param secondArg - True or False value
 * @return valid_arg_t - enum value, if the argument type is valid
 */
valid_arg_t checkValidArg(const cmd_ref_table *retCmdRef, token_t *argTok, arg_place_t argPlace, boolean_t secondArg)
{
	int i = 0;
	const type_t *validArg = argPlace == SRC_ARG ? retCmdRef->srcArg : retCmdRef->dstArg;/* returning the array of type_t
	of destnation arg or source arg */

	for (i = 0; validArg[i] != NUL_TOK; i++)
	{
		if (argTok->_type == validArg[i])/* check if type is in the valid args array*/
		{
			return VALID_ARG;
		}
	}
	printInvalidCommaArg(argTok, secondArg);
	return INVALID_ARG;
}
/**
 * @brief returning the instruction information from the op_table array
 * @param command  - array of tokens
 * @return const cmd_ref_table* -> cmd_ref_table a struct that contain the instruction information
 */
const cmd_ref_table *findCmdRef(token_t command) /* for getting the OP details*/
{
	int i = 0;
	while (i < OP_TABLE_NUM)
	{
		if (!strcmp(op_table[i].str, command.str))/* checking if reached the correct instruction info */
			return op_table + i;
		i++;
	}
	return op_table + OP_TABLE_NUM; /* contains a NULL value, for a case the instruction isnt defined*/
}

/**
 * @brief at this function we check the Syntax for all of the .somthing ... instruction
 * 
 * @param cmdLine array of token pointers, represent a line of instruction
 * @param arraylength - int 
 * @param index - int
 * @return boolean_t - TRUE or FALSE
 */
boolean_t dirRoute(token_t *cmdLine[], int arraylength, int index)
{
	int line = tokenizerState->lineNum;
	
	int flag;
	boolean_t dirOk = TRUE;

	switch (cmdLine[index]->_type)
	{
	case DATA_DIR:/* .data */
		flag = 1;
		index++;
		if (cmdLine[index++]->_type == DATA_NUM)/* if first arg is DATA_NUM*/
		{
			while (flag && index < arraylength) /* to make sure after everynumber there will be comma not including the last number*/
			{
				if (cmdLine[index++]->_type == COMMA)/* comma */
				{
					flag = 0;

					if (cmdLine[index++]->_type == DATA_NUM)/* after comma should be a datanum*/
					{
						flag = 1;
					}
					else
					{	
						if(cmdLine[index++]->_type == COMMA){
							printf("\n\033[1;31mWarning:\033[0m Line: %d => excess comma in end of line\n", line);
						}else{
							printf("\n\033[1;31mWarning:\033[0m Line: %d => illegal argument type in instruction. \n", line);
						}
						return FALSE;
					}
				}
				else
				{
					printf("\n\033[1;31mWarning:\033[0m line: %d => missing a comma. \n", line);
					return FALSE;
				}
			}
		}
		if (index != arraylength)/*  break from loop without reaching the ending of the line */
		{

			printf("\n\033[1;31mWarning:\033[0m line: %d => there are illegal argument at end of instruction. \n", line);
			return FALSE;
		}
		return TRUE;
	/* all the following cases are on the same method, cheking if the arg type is correct */
	case STRING_DIR:/* .string */
		index++;
		dirOk = checkNumArgsDir(STRING_DIR, index, arraylength, line) && 
				checkTypeArgsDir(STRING_DIR, STRING_STR, line, cmdLine[index]);
		break;

	case ENTRY_DIR: /* .entery */
		index++;
		dirOk = checkNumArgsDir(ENTRY_DIR, index, arraylength, line) && 
				checkTypeArgsDir(ENTRY_DIR, LABEL_ARG, line, cmdLine[index]);
		break;

	case EXTERN_DIR: /* .extern */
		index++;
		dirOk = checkNumArgsDir(EXTERN_DIR, index, arraylength, line) && 
				checkTypeArgsDir(EXTERN_DIR, LABEL_ARG, line, cmdLine[index]);
		break;

	default:
		return FALSE;
	}

	return dirOk;
}
/**
 * @brief return ".string" or ".entry" or ".extern" 
 * 
 * @param dirType - type_t, token type
 * @return const char* - char
 */
const char *dirStr(type_t dirType)
{
	return dirType == STRING_DIR ? ".string" : dirType == ENTRY_DIR ? ".entry" : ".extern";
}

/**
 * @brief checking the argument type
 * 
 * @param dirType - type_t, token type
 * @param argType - type_t, token type
 * @param line - int
 * @param argTok - token pointer
 * @return boolean_t - TRUE or FAlSE
 */
boolean_t checkTypeArgsDir(type_t dirType, type_t argType, int line, token_t *argTok)
{
	if (argTok->_type == argType)
		return TRUE;
	printf("\n\033[1;31mWarning:\033[0m line: %d => expected %s value for %s \n", line,
		   (argType == STRING_STR) ? " string " : " label ", dirStr(dirType));
	return FALSE;
}
/**
 * @brief checks the number of arguments
 * 
 * @param dirType - type_t => token type
 * @param index - int
 * @param arrayLength - int
 * @param line - int
 * @return boolean_t - TRUE or FALSE
 */
boolean_t checkNumArgsDir(type_t dirType, int index, int arrayLength, int line)
{
	int remainArgs = (arrayLength - index);
	if (remainArgs == 1)
		return TRUE;
	printf("\n\033[1;31mWarning:\033[0m line: %d => %s for %s \n", line,
		   remainArgs == 0 ? "dosent have enough arguments" : "there are too many args",
		   dirStr(dirType));
	return FALSE;
}