This project is a SIC/XE assembler implementation written in C. It processes SIC/XE assembly language programs and generates machine code. The assembler is divided into multiple stages, including tokenization, opcode table management, and Pass 1 and Pass 2 processing.

How to Use
1.Prepare the Assembly File
Write your SIC/XE assembly program in a text file (e.g., program.asm).

2.Compile the Assembler
Use a C compiler (e.g., GCC) to compile the source files.

gcc -o assembler token.c optable.c pass1.c pass2_SICXE.c

3.Run the Assembler
Execute the assembler with the assembly file as input.

./assembler program.asm

4.Output
The assembler will generate:
Intermediate File: Contains the symbol table and intermediate results.
Object Program: Displays the machine code in SIC/XE format.
