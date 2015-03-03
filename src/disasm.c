#include <stdio.h>
#include <stdlib.h>

#include "libarch.h"

char help_string[] = "disasm.exe <binary file name> <destination source file name>\n";

void disassembleFile(FILE *f_binary, FILE *f_destination);

int main (int argc, char *argv[])
{
    if (argc != 3)
    {
        printf(help_string);
        exit(0);
    }
    FILE *f_binary;
    FILE *f_destination;
    f_binary = fopen(argv[1], "r");
    f_destination = fopen(argv[2], "w");
    disassembleFile(f_binary, f_destination);

    return 0;
}

void disassembleFile(FILE *f_binary, FILE *f_destination)
{
    while (!feof(f_binary))
    {
        tCode code = readCodeFromBinaryFile(f_binary);
        if (code.size == 0)
            break; //finished reading -- EOF encountered
        tCommandWithOperands command = makeCommandWithOperands(code);
        writeCommandToFile(f_destination, command);
        free(code.words);
    }
    printf("Successfully disassembled file.\n");
}

