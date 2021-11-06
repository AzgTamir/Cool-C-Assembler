CFLAGS = -g -std=c90 -Wall -ansi -pedantic

OBJS = main.o tokenizer.o parser.o symTable.o syntaxChecker.o saveObj.o
INCS = parser.h data.h token.h data_defs.h saveObj.h

project:assembler

assembler : $(OBJS)
	gcc $(CFLAGS) $(OBJS) -o assembler -lm

main.o : main.c $(INCS)
	gcc $(CFLAGS) -c main.c 

parser.o : parser.c $(INCS)
	gcc $(CFLAGS) -c parser.c 

tokenizer.o : tokenizer.c $(INCS)
	gcc $(CFLAGS) -c tokenizer.c

syntaxChecker.o : syntaxChecker.c $(INCS)
	gcc $(CFLAGS) -c syntaxChecker.c

saveObj.o : saveObj.c $(INCS)
	gcc $(CFLAGS) -c saveObj.c

symTable.o : symTable.c $(INCS)
	gcc $(CFLAGS) -c symTable.c

clean:
	$(RM) project $(OBJS)

memtest:
	valgrind --leak-check=full --show-leak-kinds=all ./project ps2.as
