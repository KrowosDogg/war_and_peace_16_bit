#include "libarch.h"

#include <string.h>

#define BINARY_CODE_PRESENTATION

int operandsNumberForCommandType[COMMAND_TYPES_NUMBER] = {2, 2, 1, 1, 1, 1, 0};

char* registers[REGISTERS_NUMBER] =
     {NULL, "IP", "SP", "FLAGS", "AX", "BX", "CX", "DX"};

char* command [COMMAND_TYPES_NUMBER][MAX_COMMANDS_NUMBER_IN_TYPE] =
    {{"mov", "add", "sub", "cmp", "mul", "div", NULL},
	 {"xor", "and", "or", NULL},
	 {"not", NULL},
     {"jmp", "je", "jne", "jg", "jge", "jl", "jle", NULL},
     {"push", "pop", NULL},
     {"print", "input", NULL},
     {"hlt", NULL}
     };


tCommand parseCommand(char *cmd_name)
{
	tCommand cmd;
    for ( cmd.type = 0;
		  cmd.type < COMMAND_TYPES_NUMBER;
		  cmd.type++)
    {
        for (cmd.index = 0;
			 command [cmd.type][cmd.index] != NULL;
			 cmd.index++)
        {
            if (strcmp(command [cmd.type][cmd.index], cmd_name) == 0) //strings equal
            {
                return cmd;
            }
        }
    }
	printf("Compilation errror! cmd = %s\n", cmd_name);
	exit(-1);
}

char *commandName(tCommand cmd)
{
	return command[cmd.type][cmd.index];
}

tOperand parseOperand(char *op_name)
{
	tOperand op;
	if (op_name[0] >= '0' && op_name[0] <= '9')
	{
		op.type = NUMBER;
		op.value = atoi(op_name);
		return op;
	}
	for (op.type = 1; op.type < REGISTERS_NUMBER; op.type++)
		if (strcmp(registers[op.type], op_name) == 0)
		{
			return op;
		}
    op.type = -1;
    printf("Wrong register or number! %s\n", op_name);
    exit(-1);
    return op;
}

char buffer_for_number[MAX_OPERAND_LEXEM_LENGTH];
char * operandName(tOperand op)
{
	if (op.type == NUMBER)
	{
		snprintf(buffer_for_number, MAX_OPERAND_LEXEM_LENGTH - 1, "%d", op.value);
		return buffer_for_number;
	}
	return registers[op.type];
}

tWord makeCodeWordFromNumbers(int a, int b, int c, int d)
{
	tWord res;
	res = ((a&0xF) << 12) | ((b&0xF) << 8) | ((c&0xF) << 4) | (d&0xF);
	return res;
}

void makeNumbersFromCodeWord(tWord word, int *a, int *b,
							int *c, int *d)
{
	*a = (word >> 12)&0xF;
	*b = (word >> 8)&0xF;
	*c = (word >> 4)&0xF;
	*d = word&0xF;
}

//assembling of command with operands
tCode makeCodeFromCommandWithOperands(tCommandWithOperands cmd)
{
    tCode code;
	if (operandsNumberForCommandType[cmd.cmd.type] == 2
        && cmd.right.type == NUMBER && cmd.left.type == NUMBER)
    {
		printf("Wrong code found!\n");
		exit(-2);
	}

    code.size = 1;
	if ((operandsNumberForCommandType[cmd.cmd.type] == 1 && cmd.left.type == NUMBER) ||
        (operandsNumberForCommandType[cmd.cmd.type] == 2 && (cmd.left.type == NUMBER ||
                                                             cmd.right.type == NUMBER)))
    {
        code.size = 2;
    }
    //FIXME: what if both operands are numbers???
    code.words = (tWord *)malloc(codeSize(code));
    if (code.words == NULL)
    {
        printf("Error: No memory!\n");
        exit(-3);
    }
	code.words[0] = makeCodeWordFromNumbers(cmd.cmd.type,
										cmd.cmd.index,
										cmd.left.type,
										cmd.right.type);
	if ((operandsNumberForCommandType[cmd.cmd.type] == 1 && cmd.left.type == NUMBER) ||
        (operandsNumberForCommandType[cmd.cmd.type] == 2 && cmd.left.type == NUMBER))
    {
        code.words[1] = cmd.left.value;
    }
    else if (operandsNumberForCommandType[cmd.cmd.type] == 2 && cmd.right.type == NUMBER)
    {
        code.words[1] = cmd.right.value;
    }
    printf("debug code: %x parameter: %d\n", code.words[0], code.words[1]);
    return code;
}

//disassembling of command
tCommandWithOperands makeCommandWithOperands(tCode code)
{
    tCommand cmd;
    int left, right;
    int cmd_type;
	makeNumbersFromCodeWord(code.words[0], &cmd_type, &cmd.index, &left, &right);
	cmd.type = cmd_type;
	tCommandWithOperands cmdWithOperands;
	cmdWithOperands.cmd = cmd;
	//FIXME: maybe need to check code correctness (zeroes if no operands for the command type)
	cmdWithOperands.left.type = left;
	cmdWithOperands.right.type = right;
    if ((operandsNumberForCommandType[cmd.type] == 1 && left == NUMBER) ||
        (operandsNumberForCommandType[cmd.type] == 2 && left == NUMBER))
    {
        cmdWithOperands.left.value = code.words[1];
    }
    else if (operandsNumberForCommandType[cmd.type] == 2 && right == NUMBER)
    {
        cmdWithOperands.right.value = code.words[1];
    }
    return cmdWithOperands;
}

//returns command size in words depending on the first command word
int codeSizeByFirstWord(tWord word)
{
    int cmd_type = (word >> 12) & 0xF;
    int left_type = (word >> 4) & 0xF;
    int right_type = word & 0xF;
    if (operandsNumberForCommandType[cmd_type] == 0)
        return 1;
    else if (operandsNumberForCommandType[cmd_type] == 1)
        if (left_type == NUMBER)
            return 2;
        else
            return 1;
    else if (operandsNumberForCommandType[cmd_type] == 2)
        if (left_type == NUMBER && right_type == NUMBER)
            return 3;
        else if (left_type == NUMBER || right_type == NUMBER)
            return 2;
        else
            return 1;
    else
        return -1; //should never happens
}

//returns command size in bytes
int codeSize(tCode code)
{
    return code.size*sizeof(tWord);
}
#ifdef BINARY_CODE_PRESENTATION
//reads code from the file depending on its size
//if reading error, code.size = 0
tCode readCodeFromBinaryFile(FILE *f)
{
    tCode code;
    tWord word;
    fread(&word, sizeof(word), 1, f); //FIXME: may be reading errors
    if (word == EOF)
    {
        code.size = 0;
        return code;
    }
    code.size = codeSizeByFirstWord(word);
    code.words = (tWord*) calloc(sizeof(tWord), code.size);
    code.words[0] = word;
    for (int i = 1; i < code.size; i++)
    {
        fread(&word, sizeof(word), 1, f); //FIXME: may be reading errors
        code.words[i] = word;
    }
    return code;
}
//writes code from the file depending on its size
void writeCodeToBinaryFile(FILE *f, tCode code)
{
    fwrite(code.words, sizeof(tWord), code.size, f); //FIXME: may be writing errors
}
#else
//reads code from the file depending on its size
//if reading error, code.size = 0
tCode readCodeFromBinaryFile(FILE *f)
{
    tCode code;
    tWord word;
    //fread(&word, sizeof(word), 1, f); //FIXME: may be reading errors
    int result = fscanf(f, "%hx", &word);
    if (result == EOF || result != 1)
    {
        code.size = 0;
        return code;
    }
    code.size = codeSizeByFirstWord(word);
    code.words = (tWord*) calloc(sizeof(tWord), code.size);
    code.words[0] = word;
    for (int i = 1; i < code.size; i++)
    {
        //fread(&word, sizeof(word), 1, f); //FIXME: may be reading errors
        fscanf(f, "%hx", &word);
        code.words[i] = word;
    }
    return code;
}
//writes code from the file depending on its size
void writeCodeToBinaryFile(FILE *f, tCode code)
{
    for(int i = 0; i < code.size; i++)
        fprintf(f, "%x ", code.words[i]);
    fprintf(f, "\n");
//    fwrite(code.words, sizeof(tWord), code.size, f); //FIXME: may be writing errors
}
#endif // BINARY_CODE_PRESENTATION

tLine makeLine(tCommandWithOperands command_with_operands)
{
    tLine line;
    int operands_number = operandsNumberForCommandType[command_with_operands.cmd.type];
    if (operands_number == 0)
    {
        snprintf(line.str, MAX_COMMAND_LINE_LENGTH-1, "%s",
                 commandName(command_with_operands.cmd));
    }
    else if (operands_number == 1)
    {
        snprintf(line.str, MAX_COMMAND_LINE_LENGTH-1, "%s %s",
                 commandName(command_with_operands.cmd),
                 operandName(command_with_operands.left));
    }
    else if (operands_number == 2)
    {
        snprintf(line.str, MAX_COMMAND_LINE_LENGTH-1, "%s %s, %s",
                 commandName(command_with_operands.cmd),
                 operandName(command_with_operands.left),
                 operandName(command_with_operands.right));
    }
    return line;
}

//writes command from the file depending on its size
void writeCommandToFile(FILE *f, tCommandWithOperands command_with_operands)
{
    tLine line = makeLine(command_with_operands);
    fputs(line.str, f);
    fputc('\n', f);
}

//just returns operand number for command type id
int getOperandsNumberForCommandType(int command_type)
{
    return operandsNumberForCommandType[command_type];
}

/*
*   compare given operator_name if it is a register name
*   return boolean value -- 1 or 0
*/
int isRegisterName(char *operator_name)
{
    upperCase(operator_name);
    int op_type;
    for (op_type = 1; op_type < REGISTERS_NUMBER; op_type++)
		if (strcmp(registers[op_type], operator_name) == 0)
		{
			return 1; //true
		}
    return 0; //false
}

/*
* make all latin alphabetical symbols uppercase
*/
void upperCase(char *strpos)
{
    while (*strpos != 0) //while not end of line
    {
        if (*strpos >= 'a' && *strpos <= 'z')
        {
            *strpos += 'A' - 'a';
        }
        strpos++;
    }
}
