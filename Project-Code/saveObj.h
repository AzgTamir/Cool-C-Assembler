#ifndef _SAVE_OBJ_H_
#define _SAVE_OBJ_H_

#include <stdio.h>

#include "data.h"

/* -------------- saveObj.c --------------- */
void saveObj(asm_cmd_t CodeImage[], int instCounter, int dataCounter, char* fullFileName);
void saveSymEnt(char *srcFileName);
void saveSymExt(char *srcFileName);

#endif /* _SAVE_OBJ_H_ */