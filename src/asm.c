#include <stdio.h>

#include <stdlib.h>
#include <string.h>


#include "libarch.h"
#include "database.h"

char help_string[] = "asm.exe <source file name> <destination binary file name>\n";

void assembleFile(FILE *f_source, FILE *f_destination);

/*
*   Calculates command size in bytes assuming labels are numbers (32bit)
*   return command_size_in_bytes
*/
int calculateCommandSizeInBytes(tLine command_text);

//reads line from the file
tLine getLineFromFile(FILE *f);

//find if command is in line
int labelExistsInLine(tLine line);
//cuts off the text of command if it's exists in the line
tLine getLabelFromLine(tLine line);

//find if command is in line
int commandTextExistsInLine(tLine line);

//cuts off the label if it's exists and spaces/tabs before the command text
tLine getCommandTextFromLine(tLine line);

tCommandWithOperands readCommandFromFile(FILE *f);

//parse the part of line with the text of command with its operands
tCommandWithOperands parseCommandText(tLine command_text, tDatabase label_values);

tOperand parseLabelOperand(char *operand_name, tDatabase label_values);

/*
*   return type of operand -- number or register (correct type for each register)
*   for labels check if they correct (exit if label is not correct identifier)
*   and assumes it is just a number (32 bit)
*/
enum eRegister operandType(char *operand_name);


int main (int argc, char *argv[])
{
    printf("I'm here! FIXME\n");
    if (argc != 3)
    {
        printf(help_string);
        exit(0);
    }
    FILE *f_source;
    FILE *f_destination;
    f_source = fopen(argv[1], "r");
    f_destination = fopen(argv[2], "w");
    assembleFile(f_source, f_destination);

    return 0;
}

void assembleFile(FILE *f_source, FILE *f_destination)
{
    tDatabase label_values;
    constructLabelValuesDatabase(&label_values);

    //the first file parsing
    tAddress current_address = 0;
    while (!feof(f_source))
    {
        tLine line = getLineFromFile(f_source);
        if (labelExistsInLine(line))
        {
            tLine label = getLabelFromLine(line);
            //FIXME: check that name of label doesn't coinside with processor register name
            setLabelValue(&label_values, label, current_address);
        }
        if (commandTextExistsInLine(line))
        {
            tLine command_text = getCommandTextFromLine(line);
            int command_size_in_bytes = calculateCommandSizeInBytes(command_text); //FIXME: implement it!
            current_address += command_size_in_bytes;
        }
    }
    //for debugging print labels with their values
    printLabelValues(label_values);

    //the second file parsing
    fseek(f_source, 0, SEEK_SET); //rewind to the start of file
    while (!feof(f_source))
    {
        tLine line = getLineFromFile(f_source);
        if (commandTextExistsInLine(line))
        {
            tLine command_text = getCommandTextFromLine(line);
            tCommandWithOperands command = parseCommandText(command_text, label_values);
            tCode code = makeCodeFromCommandWithOperands(command);
            writeCodeToBinaryFile(f_destination, code);
            free(code.words);
        }
    }
    printf("Successfully assembled file.\n");
    destructLabelValuesDatabase(&label_values); //FIXME: implement it!
}

/*
*   Calculates command size in bytes assuming labels are numbers (32bit)
*   return command_size_in_bytes
*/
int calculateCommandSizeInBytes(tLine command_text)
{
    char cmd_name[MAX_COMMAND_LENGTH];
    sscanf(command_text.str, "%s", cmd_name);
    tCommand cmd = parseCommand(cmd_name);
    tCommandWithOperands command_with_operands = {cmd, {0, 0}, {0, 0}};

    int operands_number = getOperandsNumberForCommandType(cmd.type);
    if (operands_number == 0)
    {
        //just do nothing
    }
    else if (operands_number == 1)
    {
        char operand_name[MAX_OPERAND_LEXEM_LENGTH];
        sscanf(command_text.str, "%s %s", cmd_name, operand_name);
        command_with_operands.left.type = operandType(operand_name);
    }
    else if (operands_number == 2)
    {
        char operand1_name[MAX_OPERAND_LEXEM_LENGTH], operand2_name[MAX_OPERAND_LEXEM_LENGTH];
        *strchr(command_text.str, ',') = ' '; //FIXME: HARDCORE in source code...
        sscanf(command_text.str, "%s %s %s", cmd_name, operand1_name, operand2_name);
        command_with_operands.left.type = operandType(operand1_name);
        command_with_operands.right.type = operandType(operand2_name);
    }
    else //exception
    {
        printf("Wrong number of operands!!!\n");
        exit(-1);
    }

    tCode code = makeCodeFromCommandWithOperands(command_with_operands);
    int command_size_in_bytes = codeSize(code);
    free(code.words);
    return command_size_in_bytes;
}


tCommandWithOperands parseCommandText(tLine command_text, tDatabase label_values)
{
    char cmd_name[MAX_COMMAND_LENGTH];
    sscanf(command_text.str, "%s", cmd_name);
    tCommand cmd = parseCommand(cmd_name);
    tCommandWithOperands command_with_operands = {cmd, {0, 0}, {0, 0}};
    int operands_number = getOperandsNumberForCommandType(cmd.type);
    if (operands_number == 0)
        return command_with_operands;
    else if (operands_number == 1)
    {
        char operand_name[MAX_OPERAND_LEXEM_LENGTH];
        sscanf(command_text.str, "%s %s", cmd_name, operand_name);
        tOperand op = parseLabelOperand(operand_name, label_values);
        command_with_operands.left = op;
    }
    else if (operands_number == 2)
    {
        char operand1_name[MAX_OPERAND_LEXEM_LENGTH], operand2_name[MAX_OPERAND_LEXEM_LENGTH];
        *strchr(command_text.str, ',') = ' '; //FIXME: HARDCORE in source code...
        sscanf(command_text.str, "%s %s %s", cmd_name, operand1_name, operand2_name);
        tOperand op1 = parseLabelOperand(operand1_name, label_values);
        command_with_operands.left = op1;
        tOperand op2 = parseLabelOperand(operand2_name, label_values);
        command_with_operands.right = op2;
    }
    else //exception
    {
        printf("Wrong number of operands!!!\n");
        exit(-1);
    }
    return command_with_operands;
}

tOperand parseLabelOperand(char *operand_name, tDatabase label_values)
{
    tOperand operand;
    tAddress value = getLabelValue(operand_name, label_values);
    if (value == -1)
    {
        operand = parseOperand(operand_name);
    }
    else
    {
        operand.type = 0;
        operand.value = value;
    }
    return operand;
}

//reads line from the file
tLine getLineFromFile(FILE *f)
{
    tLine line;
    fgets(line.str, MAX_COMMAND_LINE_LENGTH-1, f);
    return line;
}

//find if label is in line
int labelExistsInLine(tLine line)
{
    char *pos = strchr(line.str, ':');
    //symbol ';' means comment in the line with command
    char *commentpos = strchr(line.str, ';');

    if (pos == NULL || (commentpos != NULL && pos > commentpos))
        return 0; //false
    else
        return 1; //true
}

/** \brief cuts off the text of command if it's exists in the line
 *
 * \param line as is
 * \return label text without any spaces around it
 *
 */
tLine getLabelFromLine(tLine line)
{
    if (!labelExistsInLine(line))
    {
        printf("error! trying to get label from line without label!\n");
        exit(3);
    }
    tLine label;
    char *endpos = strchr(line.str, ':');
    //copying line from the beginning to the ':' symbol
    char *linepos = line.str;
    char *labelpos = label.str;
    while (linepos != endpos)
    {
        *labelpos = *linepos;
        labelpos++;
        linepos++;
    }
    *labelpos = 0;
    return label;
}

//bool function really
int commandTextExistsInLine(tLine line)
{
    char *commandpos;
    if (labelExistsInLine(line))
        commandpos = strchr(line.str, ':') + 1;
    else
        commandpos = line.str;
    while (*commandpos != 0 && *commandpos != ';') //symbol ';' means comment in the line with command
    {
        if (*commandpos != ' ' && *commandpos != '\t' && *commandpos != '\n')
            return 1; //true -- encountered nonvoid symbol after label
        commandpos++;
    }
    return 0; //false
}

//cuts of the label if it's exists and spaces/tabs before the command text
tLine getCommandTextFromLine(tLine line)
{
    char *commandpos;
    if (labelExistsInLine(line))
        commandpos = strchr(line.str, ':') + 1;
    else
        commandpos = line.str;
    //skip spaces before significant character (the start of command)
    while (*commandpos == ' ' || *commandpos == '\t')
        commandpos++;

    tLine command_text;
    //copying line from the commandpos to the ';' symbol or to the end
    char *pos = command_text.str;
    while (*commandpos != ';' && *commandpos != 0)
    {
        *pos = *commandpos;
        pos++;
        commandpos++;
    }
    *pos = 0;
    return command_text;
}

/*
*   return type of operand -- number or register (correct type for each register)
*   for labels check if they correct (exit if label is not correct identifier)
*   and assumes it is just a number (32 bit)
*/
enum eRegister operandType(char *operand_name)
{
    //FIXME
    if (isRegisterName(operand_name))
    {
         return 1;
    }
    else
    {
        return 0;
    }
}
