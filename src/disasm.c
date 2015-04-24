#include <stdio.h>
#include <stdlib.h>

#include "libarch.h"
#include "database.h"

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

/*
    checks if command type is JUMP_OP
    returns true (1) of false (0)
*/
int isJumpCommand(tCommandWithOperands command);

//writes command from the file depending on its size
void writeCommandToFile(FILE *f, tCommandWithOperands command,
                        tAddressDatabase addresses_values);


void disassembleFile(FILE *f_binary, FILE *f_destination)
{
    tAddressDatabase addresses_values;
    constructAddressesDatabase(&addresses_values);

    //the first file parsing
    tAddress current_address = 0;
    while (!feof(f_binary))
    {
        tCode code = readCodeFromBinaryFile(f_binary);
        if (code.size == 0)
            break; //finished reading -- EOF encountered
        tCommandWithOperands command = makeCommandWithOperands(code);
        if (isJumpCommand(command))
        { //FIXME: implement!
            storeAddress(&addresses_values, command.left.value);
        }
        current_address += codeSize(code);
        free(code.words);
    }

    generateAddressesLabels(addresses_values);
    //for debugging: print values with generated addresses
    printAddressesValues(addresses_values);

    //the second file parsing
    fseek(f_binary, 0, SEEK_SET); //rewind to the start of file
    current_address = 0;
    while (!feof(f_binary))
    {
        tCode code = readCodeFromBinaryFile(f_binary);
        if (code.size == 0)
            break; //finished reading -- EOF encountered
        tCommandWithOperands command = makeCommandWithOperands(code);

        if (addressInAddressesBase(current_address, addresses_values))
        {
            tLine label = getAddressLabel(current_address, addresses_values);
            fprintf(f_destination, "%s:\t", label.str);
        }
        else
        {
            fprintf(f_destination, "\t");
        }

        writeCommandToFile(f_destination, command, addresses_values);
        current_address += codeSize(code);
        free(code.words);
    }

    printf("Successfully disassembled file.\n");
}

int isJumpCommand(tCommandWithOperands command)
{
    if (command.cmd.type == JUMP_OP)
        return 1; //true
    else
        return 0; //false;
}

//writes command from the file depending on its size
void writeCommandToFile(FILE *f, tCommandWithOperands command_with_operands,
                        tAddressDatabase addresses_values)
{
    tLine line;
    if (isJumpCommand(command_with_operands) && command_with_operands.left.type == NUMBER)
    {
        snprintf(line.str, MAX_COMMAND_LINE_LENGTH-1, "%s %s",
            commandName(command_with_operands.cmd),
            getAddressLabel(command_with_operands.left.value, addresses_values).str);
    }
    else
    {
        line = makeLine(command_with_operands);
    }
    fputs(line.str, f);
    fputc('\n', f);
}

