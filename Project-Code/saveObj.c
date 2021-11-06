#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "data.h"
#include "symTable.h"


static int mainPrintPerLine(asm_cmd_t asmCmd[], int instCounter, int dataCounter, FILE* obFile);



 /**
  * @brief this function is the main function that write and create the ob file,
  *         her parameters are the array of each line code image
  * 
  * @param srcFileName the original file name
  * @param newExt the new extended file name
  * @return FILE* return the new file with the wanted extention
  */
FILE *openWithExt(char *srcFileName, char *newExt) {
    FILE * obFile;

    char *ext = strchr(srcFileName, '.');
    strcpy(ext+1, newExt);
    if((obFile = fopen(srcFileName, "wt")) == NULL)
    {
        printf("\n\033[1;31mWarning:\033[0m cannot open or create file named as %s \n", srcFileName);
        return NULL;
    }
    return obFile;
}

/**
 * @brief creating entry file
 * 
 * @param srcFileName the original file name
 */
void saveSymEnt(char *srcFileName) {
    FILE *entFile = openWithExt(srcFileName, "ent");
    if (entFile == NULL) return;
    printSymEnts(entFile); /*writing into .ent file (symTable.c) */
    fclose(entFile);
}

/**
 * @brief creating extern file 
 *  
 * @param srcFileName the original file name 
 */
void saveSymExt(char *srcFileName) {
    FILE *extFile = openWithExt(srcFileName, "ext");
    if (extFile == NULL) return;
    printSymExts(extFile); /*writing into .ext file (symTable.c)*/
    fclose(extFile);
}

/**
 * @brief creating object file  
 * 
 * @param asmCmd the processed code
 * @param instCounter instraction counter 
 * @param dataCounter  data image counter
 * @param srcFileName original file name
 */
void saveObj(asm_cmd_t asmCmd[], int instCounter, int dataCounter, char* srcFileName) {
    FILE *obFile = openWithExt(srcFileName, "ob");
    if (obFile == NULL) return;
    mainPrintPerLine(asmCmd, instCounter, dataCounter, obFile);
    fclose(obFile);
}

/**
 * @brief writing into file .ob
 * 
 * @param asmCmd  the processed code
 * @param instCounter instraction counter
 * @param dataCounter  data image counter
 * @param obFile original file name
 * @return int 
 */
int mainPrintPerLine(asm_cmd_t asmCmd[], int instCounter, int dataCounter, FILE* obFile){
    int i;
    /* printing the instCounter and dataCounter */
    fprintf(obFile, "%7d %d\n", instCounter-dataCounter, dataCounter); 

    for(i=0; i < instCounter; i++){
        /* printing in the format
            (address) (machine code in hexadecimal) 
            for example: 0000020 00ab45 => 
            address = 20 & machine code in hexadecimal = ab45 */
        fprintf(obFile,"%07d  %02x%02x%02x \n", i+BASE_INST_COUNTER, 
                    asmCmd[i]._binCmd._bin_cmd[2],
                    asmCmd[i]._binCmd._bin_cmd[1],
                    asmCmd[i]._binCmd._bin_cmd[0] );
    }
    return 0;
}