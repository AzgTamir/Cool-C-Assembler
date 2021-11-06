#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "symTable.h"
#include "token.h"
#include "data.h"
#include "parser.h"

sym_table_t *labelTable;
int labelTableSize;
int labelNum;

/*extern:*/
/*void addLabelDef(char* str, int instCounter, sym_attr_t labelAttr);*/
static void newLabelDef(char *str, int instCounter, sym_attr_t labelAttr, int line);
static void setLabelDef(int i, int instCounter, sym_attr_t labelAttr);

/*extern:*/
/*void addLabelArgAddr(char* str, sym_attr_t labelAttr);*/
/*void addLabelArg(char* str, sym_attr_t labelAttr);*/
static int newLabelArg(char *str, sym_attr_t labelAttr);

/**
 * @brief initiolizing and memsetting the label table
 * 
 */
void initLabelTable()
{
    static boolean_t first=TRUE;
    if (first) { 
        labelTable = NULL;
        first = FALSE;
    }

    labelTableSize = MAX_LABELS;
    labelTable = realloc(labelTable, labelTableSize * sizeof(sym_table_t));
    memset(labelTable, 0, labelTableSize * sizeof(sym_table_t)); 
    labelNum = 0;
}

/**
 * @brief  cleaning previouse lables  
 * 
 */
void destroyLabelTable() { 
    free(labelTable);
}

/**
 * @brief making sure theres enough memory for additional labels
 * 
 * @return boolean_t return if succeed
 */
boolean_t checkLabelTableSize() { 
    if (labelNum + 2 > labelTableSize) {
        sym_table_t *tmp;
        int new_size = (labelTableSize + MAX_LABELS) * sizeof(sym_table_t);
        tmp = realloc(labelTable, new_size);
        if (tmp != NULL) {
            labelTable = tmp;
            memset(labelTable + labelTableSize, 0, MAX_LABELS * sizeof(sym_table_t));
            labelTableSize += MAX_LABELS;
            printf("Note: out of memory in line: %d, current labelTable size: %d, succesfully increased.\n", prsState->_line, labelTableSize);
        }
        else{
            printf("Fatal: out of memory in line: %d, current labelTable size: %d \n", prsState->_line, labelTableSize);
            exit(-1);
        }
    }
    return TRUE;
}

/**
 * @brief re-set the label definition (if there was any declaration of it previously) 
 * 
 * @param i the index to the label table
 * @param instCounter the instraction counter to set the label address offset
 * @param labelAttr the current label attribute
 */
void setLabelDef(int i, int instCounter, sym_attr_t labelAttr)
{
    labelTable[i].addr = instCounter + BASE_INST_COUNTER;
    /*label's attribute sets local if there was not any declaration of directive in relation to this label*/
    if (labelTable[i].LabelAttr == SYM_UNDEFINED) 
    {
        labelTable[i].LabelAttr = labelAttr;
    }
    labelTable[i].line = prsState->_line;
}

/**
 * @brief finding the label in the label Tabel by the label's string
 * 
 * @param str the given label string
 * @return int the index to the currect label (-1 if not found)
 */
int getLabel(char *str)
{
    int i = 0;
    for (; i < labelNum; i++)
    {
        if (!strcmp(str, labelTable[i].symStr))
        {
            return i;
        }
    }
    return -1;
}

/**
 * @brief creating new label definition-type to the label tabel
 * 
 * @param str the label's string
 * @param instCounter current instraction counter
 * @param labelAttr current attribute (label definition - always local)
 * @param line curretn line number
 */
void newLabelDef(char *str, int instCounter, sym_attr_t labelAttr, int line)
{
    if (checkLabelTableSize()) /* checkin for label overflow - exit if realloc faild*/
    {
        strcpy(labelTable[labelNum].symStr, str); /*adds without the ":" */
        labelTable[labelNum].addr = instCounter + BASE_INST_COUNTER;
        labelTable[labelNum].LabelAttr = labelAttr; 
        labelTable[labelNum].line = line;
        labelNum++;
    }

}

/**
 * @brief creating new label argument-type to the label tabel
 * 
 * @param str the label's string
 * @param labelAttr current attribute (extern, lacal, entry)
 * @return int return the new label index 
 */
int newLabelArg(char *str, sym_attr_t labelAttr)
{
    int labelIndex = -1;
    if (checkLabelTableSize()) /* checkin for label overflow - exit if realloc faild*/
    {                                             
        strcpy(labelTable[labelNum].symStr, str); /* add empty label  */
        labelTable[labelNum].addr = 0;
        labelTable[labelNum].LabelAttr = labelAttr;
        labelTable[labelNum].line = 0;
        labelIndex = labelNum;
        labelNum++;
    }

    return labelIndex;
}

/**
 * @brief  main function of handeling label-definition  
 * 
 * @param str the label's string
 * @param instCounter current instraction counter
 * @param labelAttr current attribute - always local
 */
void addLabelDef(char *str, int instCounter, sym_attr_t labelAttr)
{
    int i = getLabel(str);
    /*------------- check for "full" existing label - if a label exist but without values, then enter values----------*/
    if (i >= 0)
    {
        if (labelTable[i].addr || (labelTable[i].LabelAttr == SYM_EXTERN))
        { /* fully defined */
            printf("\n\033[1;31mWarning:\033[0m Line: %d => duplicate label definition, label: '%s' \n",  prsState->_line, str);
            prsState->_error++;
        }
        else
        {
            setLabelDef(i, instCounter, labelAttr);
        }
    }
    else
    {
        /*label doesnt exist --> adds label*/
        newLabelDef(str, instCounter, labelAttr, prsState->_line);
    }
}


/**
 * @brief main function of handeling label-argument 
 * 
 * @param str labels' string 
 * @param labelAttr current attribute (extern, lacal, entry)
 */
void addLabelArg(char *str, sym_attr_t labelAttr)
{
    int i = getLabel(str); /*seaching for label*/
    if (i < 0)
    { /* --> label doesnt exist */
        /* adds the label to the label tabel (still unknown values) */
        i = newLabelArg(str, labelAttr);
        /* the label will be added to the binary code after this function*/
    }

    if (labelAttr == SYM_ENTRY || labelAttr == SYM_EXTERN )
    {
        if( labelTable[i].LabelAttr == SYM_LOCAL && labelAttr == SYM_EXTERN ){
            printf("\n\033[1;31mWarning:\033[0m Line: %d => can't set extern attribute to local label", prsState->_line);
            prsState->_error++;
        }
        else labelTable[i].LabelAttr = labelAttr;
    }
}

/**
 * @brief main function of handeling label-address 
 * 
 * @param str labels' string 
 * @param labelAttr current attribute (extern, lacal, entry)
 */
void addLabelArgAddr(char *str, sym_attr_t labelAttr)
{
    int i = getLabel(str); 
    if (i < 0)
    { /* --> label doesnt exist */
        /* adds the label to the label tabel (still unknown values) */
        newLabelArg(str, labelAttr);
        /* the label will be added to the binary code after this function*/
    }
}

/**
 * @brief prints the label table 
 * 
 */
void printSymTable()
{
    int i = 0;
    for (i = 0; i < labelNum; i++)
    {
        printf("%2d. %-12s %2d %6d\n", i, labelTable[i].symStr, labelTable[i].LabelAttr, labelTable[i].addr);
    }
}


/**
 * @brief 
 * 
 * @param a 
 * @param b 
 * @return int 
 */
int labelEntryCmp(const void *a, const void *b)
{
    const sym_table_t *a_l = (sym_table_t *)a;
    const sym_table_t *b_l = (sym_table_t *)b;
    return (a_l->addr < b_l->addr) ? -1 : (a_l->addr == b_l->addr) ? 0 : 1;
}

/**
 * @brief writes entry labels and reorgnize label table according to its address (lowest to highest)
 * 
 * @param entFile the file to be written in
 */
void printSymEnts(FILE *entFile)
{
    int i = 0;
    qsort(labelTable, labelNum, sizeof(sym_table_t), labelEntryCmp);

    for (i = 0; i < labelNum; i++)
    {
        if (labelTable[i].LabelAttr == SYM_ENTRY)
            fprintf(entFile, "%s %07d\n", labelTable[i].symStr, labelTable[i].addr);
    }
}

/**
 * @brief writes entry labels (label table already orgenized (printSymEnts called first in  main.c ))
 * 
 * @param extFile the file to be wrriten in
 */
void printSymExts(FILE *extFile)
{
    int i = 0;
    for (i = 0; i < prsState->_instCounter; i++)
    {
        if ((asmCmd[i]._type == LABEL_ARG || asmCmd[i]._type == LABEL_ADDR) &&
            asmCmd[i]._binCmd._bin_arg.ARE == EXTERNAL_ARE)
        {
            fprintf(extFile, "%s %07d\n", asmCmd[i]._labelStr, i + BASE_INST_COUNTER);
        }
    }
}
