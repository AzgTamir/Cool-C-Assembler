#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "syntaxChecker.h"
#include "token.h"
#include "parser.h"
#include "symTable.h"
#include "saveObj.h"

static void handle_file(char *filename);
char *checkValidExt(char *filename);

int main(int argc, char *argv[])
{
    void printSymTable();
    int i;
    for (i = 1; i < argc; i++)
    {
        printf("\n-------------- Working on File: %s ------------- \n",argv[i] );
        handle_file(argv[i]);
    }

    destroyLabelTable();
    destroyParserState();
    return 0;
}

/**
 * @brief make sure file name will contian .as + one more space for .ext & .ent
 * 
 * @param filename 
 * @return char* - the file name with .as
 */
char *checkValidExt(char *filename) { /*make sure file name will contian .as + one more space for .ext & .ent*/
    int fileLen = strlen(filename);
    char *tmp = malloc(fileLen+4);
    strcpy(tmp, filename);
    if (strstr(tmp, ".as") == NULL) { 
        strcat(tmp, ".as");
    }
    return tmp;
}

/**
 * @brief main function to handel the file and manipulating the file throught the functions
 * 
 * @param filename the current file
 */
void handle_file(char *filename) 
{
    char *srcFile = checkValidExt(filename); /*making sure theres .as at end of file name*/

    FILE *fp = fopen(srcFile, "rt");
    char lineStr[MAX_LINE_BUFLEN + 1]; /* take +1 margin to allow padding with \n */
    int lineNum = 1;
    int errNum = 0;
    if (fp == NULL)
    {
        perror(srcFile);
        printf("\033[1;31mWarning:\033[0m cannot open or create file named as %s \n", srcFile);
        return;
    }

    /*initiolazing states - see more about each state in thier files*/
    initLabelTable(); /*symTable.c*/
    initTokenizerState();/*tokenizer.c*/
    initParserState();/*parser.c*/


    for (lineNum = 1; !feof(fp); lineNum++)
    {
        printf("\n--------------- Line: %d ---------------\n", lineNum); /*to your pereference*/
        fgets(lineStr, MAX_LINE_LEN, fp);
        if (!checkValidLine(fp, lineStr)) /*source input support functions for tokenizer (setting \ n at end of line )*/
        {
            discardInputLine(fp, lineStr); /*if line is more then 80 characters*/
            errNum++;

            printf("\n\033[1;31mNot proceessing line: %d due to invalid line length error \033[0m \n", lineNum);

            continue;
        }

        clearTokenizerLine(); /* cleaning tokenizerState attributes - see more about tokenaizerState in Tokenizer.c*/
        clearParserError();

        lineToTokens(lineNum, lineStr); /* main function seperate to two missions: 1.seperate line to words 2.tag each word accordingly*/
        if (checkTokenizerErrors()) /*checking for erroes in the lineToToken transsition */
        {
            printf("\n\033[1;31mNot proceessing line: %d due to tokenizer error\033[0m\n", lineNum);
            errNum += tokenizerState->errors;
            continue;
        }
        else if (tokenizerState->numOfTokens == 0)
            continue;

        if (!syntaxChecker(lineNum, tokenizerState->tokenList, tokenizerState->numOfTokens))/* checking for syntax error */
        {
           printf("\n\033[1;31mNot proceessing line: %d due to syntax error\033[0m\n", lineNum);
            errNum++;
            continue;
        }
        parsePass1(lineNum, tokenizerState->tokenList, tokenizerState->numOfTokens); /* first pass - start coding each "token" to his binary value 
                                                                                        (coding "eampty" binary line to undefined labels) */
        if(checkPraserErrors()){ /*checking for erroes in the Parser transsition */

            printf("\n\033[1;31mNot proceessing line: %d due to parser error\033[0m\n", lineNum);

            errNum += prsState->_error;
            continue;
        }
    
    }


    errNum += parsePass2(); /* Second pass - compliting missing coding infromation and value to undefined labels*/

    if (!errNum) { /*coding to file only if there was no warnings*/
        saveObj(asmCmd, prsState->_instCounter, prsState->_dataCounter, srcFile);
        saveSymEnt(srcFile); /* calling saveSymEnt to orgenize label table (symTable.c)*/
        saveSymExt(srcFile); /* (symTable.c) */
    }  else {
        printf("\033[1;31mWarning:\033[0m not generating binary files for %s, with %d errors.\n", srcFile, errNum);
    }

    
    if (!errNum) { /* printing binary file code only if there was no warnings - to your pereference */
        printf("\n ----------- Printing code and labels valeus - code was written to files ------------- \n");
        parserAsmCmdPrint();
        printSymTable();
    }
    
    /*clearTokenizerBLine();*/
    free(srcFile); 
    fclose(fp);
}