# Cool-C-Assembler
Project Structure:
The project is divided into two main segments: Tokenizer and the Parser.
The tokenizer’s job is to tag each word to its type, for example:
mov r1, r2.
will be tagged as:
OP - REG - REG
so we can easlly check for syntax and type errors.
Next, the parser will code each line into the binaries. The parser will scan through the token table twise to resulve label value and address.

