#ifndef LIBARCH_H
#define LIBARCH_H

#include <stdlib.h>
#include <stdio.h>

//16-bit architecture!!!
#define WORD_SIZE 2
typedef unsigned short int tWord;
typedef unsigned char tByte;
typedef long int tAddress; //only 16 bit values should be stored here! -1 means NULL

#define MAX_CODE_LENGTH 2
#define MAX_COMMAND_LENGTH 5
#define MAX_COMMANDS_NUMBER_IN_TYPE 8
#define MAX_COMMAND_LINE_LENGTH 100
#define MAX_OPERAND_LEXEM_LENGTH 50

#define COMMAND_TYPES_NUMBER 7
enum eCommandType {ARITHMETICAL_OP, LOGICAL_BINARY_OP, LOGICAL_UNARY_OP, JUMP_OP, STACK_OP, IO_OP, HLT_OP};

#define REGISTERS_NUMBER 8
enum eRegister {NUMBER, IP, SP, FLAGS, AX, BX, CX, DX};

typedef struct
{
    enum eCommandType type;
	int index;
} tCommand;

tCommand parseCommand(char *cmd_name);

char *commandName(tCommand cmd);

typedef struct
{
	enum eRegister type;
	tWord value; //if type == NUMBER
} tOperand;

//find operand name in the list of operands name or finds its a number
tOperand parseOperand(char *op_name);
char * operandName(tOperand op);

typedef struct
{
    tWord *words;
	size_t size;
} tCode;

typedef struct
{
	tCommand cmd;
	tOperand left, right;
} tCommandWithOperands;

typedef struct
{
	char str[MAX_COMMAND_LINE_LENGTH];
} tLine;

//assembling of command
tCode makeCodeFromCommandWithOperands(tCommandWithOperands cmd);
//disassembling of command
tCommandWithOperands makeCommandWithOperands(tCode code);

//returns command size in words depending on the first command word
int codeSizeByFirstWord(tWord word);

//returns command size in bytes
int codeSize(tCode code);

//reads code from the file depending on its size
tCode readCodeFromBinaryFile(FILE *f);
//writes code from the file depending on its size
void writeCodeToBinaryFile(FILE *f, tCode code);

tLine makeLine(tCommandWithOperands cmd);

//just returns operand number for command type id
int getOperandsNumberForCommandType(int command_type);

/*
*   compare given operator_name if it is a register name
*   return boolean value -- 1 or 0
*/
int isRegisterName(char *operator_name);

/*
* make all latin alphabetical symbols uppercase
*/
void upperCase(char *strpos);

#endif // LIBARCH_H
