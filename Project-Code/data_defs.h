
#ifndef _DATA_DEFS_H_
#define _DATA_DEFS_H_

#define MAX_ASM_CMD 1000        /* memory map limit */
#define MAX_LABELS  100         /* symbol table limit*/
#define BASE_INST_COUNTER 100  /* memory map base address */

#define MAX_LINE_LEN    80     /* tokenizer source line limit */
#define MAX_LINE_BUFLEN (MAX_LINE_LEN+2) /* tokenizer source line limit */
#define MAX_LINE_TOKENS 82     /* tokenizer token count limit */

#define SYM_NAME_LEN (MAX_LINE_LEN) /* tokenizer source symbol name limit */
#define MAX_ASM_PRINT_LEN 200
#define OP_TABLE_NUM (OPS_NUM + 1) /* OPS_NUM is enum - post-compile time, defined as a macro */


#endif /*_DATA_DEFS_H_*/