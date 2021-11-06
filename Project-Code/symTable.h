#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "data.h"


/* --------- symTable.c ------------- */
void addLabelDef(char *str, int instCounter, sym_attr_t labelAttr);
void addLabelArg(char *str, sym_attr_t labelAttr);
void addLabelArgAddr(char *str, sym_attr_t labelAttr);

void initLabelTable();
void destroyLabelTable();

void printSymTable();
int getLabel(char *str);

void printSymEnts(FILE *entFile);
void printSymExts(FILE *extFile);

#endif /*SYMTABLE_H*/