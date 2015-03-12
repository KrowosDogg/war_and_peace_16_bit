#include "libarch.h"

#include <stdio.h>
#include <string.h>

#define RAM_SIZE 64*1024

char init_memory_state_filename[] = "ram_init.txt";
char final_memory_state_filename[] = "ram_final.txt";


void _mov (tCommandWithOperands cmd);
void _add (tCommandWithOperands cmd);
void _sub (tCommandWithOperands cmd);
void _cmp (tCommandWithOperands cmd);
void _mul (tCommandWithOperands cmd);
void _div (tCommandWithOperands cmd);
void _xor (tCommandWithOperands cmd);
void _and (tCommandWithOperands cmd);
void _or (tCommandWithOperands cmd);
void _not (tCommandWithOperands cmd);
void _jmp (tCommandWithOperands cmd);
void _je (tCommandWithOperands cmd);
void _jne (tCommandWithOperands cmd);
void _jg (tCommandWithOperands cmd);
void _jge (tCommandWithOperands cmd);
void _jl (tCommandWithOperands cmd);
void _jle (tCommandWithOperands cmd);
void _push (tCommandWithOperands cmd);
void _pop (tCommandWithOperands cmd);
void _print (tCommandWithOperands cmd);
void _input (tCommandWithOperands cmd);
void _hlt (tCommandWithOperands cmd);

void (*commandAction [COMMAND_TYPES_NUMBER][MAX_COMMANDS_NUMBER_IN_TYPE])(tCommandWithOperands) =
    {{_mov, _add, _sub, _cmp, _mul, _div, NULL},
	 {_xor, _and, _or, NULL},
	 {_not, NULL},
     {_jmp, _je, _jne, _jg, _jge, _jl, _jle, NULL},
     {_push, _pop, NULL},
     {_print, _input, NULL},
     {_hlt, NULL}
     };

tByte RAM [RAM_SIZE];
tWord regValue[REGISTERS_NUMBER] = {0};
int processing_halted_flag = 0; //HLT command set this flag to 1
int command_did_jump = 0; //If any jumping was done this flag is set to 1 for awhile

//allocates memory for the code.words
tCode readCodeFromRAM(tByte RAM[], tWord address);

void writeCodeToRAM(tByte RAM[], tWord address, tCode code);

tWord getOperandValue(tOperand operand);

enum FlagType {CarryFlag = 1, NegativeFlag = 2, ZeroFlag = 4};
int getFlagValue(enum FlagType flagType);

void executeFile(tByte RAM[], const char *binary_filename);
void saveMemoryToFile(tByte RAM[], const char *final_memory_state_filename);

char help_string[] = "processor.exe <binary file name>\n"
                     "   interprets binary.\n";

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf(help_string);
        exit(0);
    }
    char *file_to_execute = argv[1];
    printf("Executing file %s\n", file_to_execute);
    executeFile(RAM, file_to_execute);
    while (!processing_halted_flag)
    {
        tCode code = readCodeFromRAM (RAM, regValue[IP]);
        tCommandWithOperands cmd = makeCommandWithOperands(code);

        commandAction[cmd.cmd.type][cmd.cmd.index](cmd);

        if (command_did_jump)
            command_did_jump = 0;
        else
            regValue[IP] += code.size * WORD_SIZE;
        free(code.words);
    }
    saveMemoryToFile(RAM, final_memory_state_filename);
    return 0;
}

tCode readCodeFromRAM(tByte RAM[], tWord address)
{
    tCode code;
    tWord word;
    memcpy(&word, &RAM[address], WORD_SIZE);
    code.size = codeSizeByFirstWord(word);
    code.words = (tWord*) calloc(sizeof(tWord), code.size);
    code.words[0] = word;
    for (int i = 1; i < code.size; i++)
    {
        memcpy(&word, (tWord*)&RAM[address] + i, WORD_SIZE);
        code.words[i] = word;
    }
    return code;
}

void writeCodeToRAM(tByte RAM[], tWord address, tCode code)
{
    memcpy (&RAM[address], code.words, WORD_SIZE*code.size);
}

void executeFile(tByte RAM[], const char *binary_filename)
{
    int address = 0;
    FILE *f_binary = fopen(binary_filename, "r");
    if (f_binary == NULL)
    {
        printf("Can't load file! No such file.\n");
        exit(1);
    }
    while (!feof(f_binary))
    {
        tCode code = readCodeFromBinaryFile(f_binary);
        if (code.size == 0)
            break; //finished reading -- EOF encountered
        writeCodeToRAM(RAM, address, code);
        address += WORD_SIZE * code.size;
        free(code.words);
    }
    fclose(f_binary);
    printf("File loaded to RAM.\n");
}

void saveMemoryToFile(tByte RAM[], const char *final_memory_state_filename)
{
    FILE *f_memory_out = fopen(final_memory_state_filename, "w");
    fwrite(RAM, sizeof(tByte), RAM_SIZE, f_memory_out);
    fclose(f_memory_out);
}

//get operand value
tWord getOperandValue(tOperand operand)
{
    return (operand.type == 0) ? operand.value : regValue[operand.type];
}

int getFlagValue(enum FlagType flagType)
{
    return !!((int)(regValue[FLAGS]%256) & flagType);
    //double logical NOT will make 1 from any nonzero and 0 from 0
}

void _mov (tCommandWithOperands cmd)
{
    regValue[cmd.left.type] = getOperandValue(cmd.right);
}

void _add (tCommandWithOperands cmd)
{
    int result = (int)regValue[cmd.left.type] + (int)getOperandValue(cmd.right);
    regValue[cmd.left.type] = (tWord)result;
    regValue[FLAGS] = CarryFlag*((result & (~0xFFFF)) != 0) | ZeroFlag*(result == 0);
}

void _sub (tCommandWithOperands cmd)
{
    int result = (int)regValue[cmd.left.type] - (int)getOperandValue(cmd.right);
    regValue[cmd.left.type] = (tWord)result; //FIXME: to document what is result if negative value
    regValue[FLAGS] = NegativeFlag*(result < 0) | CarryFlag*((result & ~0xFFFF) != 0) | ZeroFlag*(result == 0);
}

void _cmp (tCommandWithOperands cmd)
{
    int result = (int)regValue[cmd.left.type] - (int)getOperandValue(cmd.right);
    regValue[FLAGS] = NegativeFlag*(result < 0) | CarryFlag*((result & ~0xFFFF) != 0) | ZeroFlag*(result == 0);
}

void _mul (tCommandWithOperands cmd)
{
    int result = (int)regValue[cmd.left.type] * (int)getOperandValue(cmd.right);
    regValue[cmd.left.type] = (tWord)result;
    regValue[FLAGS] = CarryFlag*((result & ~0xFFFF) != 0) | ZeroFlag*(result == 0);
}

//ѕосле операции частное помещаетс€ в ax, а остаток - в dx.
void _div (tCommandWithOperands cmd)
{
    tWord left = getOperandValue(cmd.left);
    tWord right = getOperandValue(cmd.right);
    if (right == 0)
    {
        printf("ERROR: Division by zero!");
        _hlt(cmd);
    }
    regValue[AX] = left / right;
    regValue[DX] = left % right;
    regValue[FLAGS] = ZeroFlag*(regValue[AX] == 0);
}

void _xor (tCommandWithOperands cmd)
{
    tWord result = regValue[cmd.left.type] ^ getOperandValue(cmd.right);
    regValue[cmd.left.type] = (tWord)result;
    regValue[FLAGS] = ZeroFlag*(result == 0);
}

void _and (tCommandWithOperands cmd)
{
    tWord result = regValue[cmd.left.type] & getOperandValue(cmd.right);
    regValue[cmd.left.type] = (tWord)result;
    regValue[FLAGS] = ZeroFlag*(result == 0);
}

void _or (tCommandWithOperands cmd)
{
    tWord result = regValue[cmd.left.type] | getOperandValue(cmd.right);
    regValue[cmd.left.type] = (tWord)result;
    regValue[FLAGS] = ZeroFlag*(result == 0);
}

void _not (tCommandWithOperands cmd)
{
    tWord result = ~regValue[cmd.left.type];
    regValue[cmd.left.type] = (tWord)result;
    regValue[FLAGS] = ZeroFlag*(result == 0);
}

void _jmp (tCommandWithOperands cmd)
{
    regValue[IP] = getOperandValue(cmd.left);
    command_did_jump = 1;
}

void _je (tCommandWithOperands cmd)
{
    if (getFlagValue(ZeroFlag) == 1) {
        regValue[IP] = getOperandValue(cmd.left);
        command_did_jump = 1;
    }
}
void _jne (tCommandWithOperands cmd)
{
	if (getFlagValue(ZeroFlag) == 0) {
        regValue[IP] = getOperandValue(cmd.left);
        command_did_jump = 1;
    }
}

void _jg (tCommandWithOperands cmd)
{
	if (getFlagValue(ZeroFlag) == 0 && getFlagValue(NegativeFlag) == 0) {
        regValue[IP] = getOperandValue(cmd.left);
        command_did_jump = 1;
    }
}

void _jge (tCommandWithOperands cmd)
{
	if (getFlagValue(NegativeFlag) == 0) {
        regValue[IP] = getOperandValue(cmd.left);
        command_did_jump = 1;
    }
}

void _jl (tCommandWithOperands cmd)
{
	if (getFlagValue(NegativeFlag) == 1) {
        regValue[IP] = getOperandValue(cmd.left);
        command_did_jump = 1;
    }
}

void _jle (tCommandWithOperands cmd)
{
	if (getFlagValue(ZeroFlag) == 1 || getFlagValue(NegativeFlag) == 1) {
        regValue[IP] = getOperandValue(cmd.left);
        command_did_jump = 1;
    }
}

void _push (tCommandWithOperands cmd)
{
	tWord value = getOperandValue(cmd.left);
	regValue[SP] -= WORD_SIZE;
	memcpy(&RAM[regValue[SP]], &value, WORD_SIZE);
}

void _pop (tCommandWithOperands cmd)
{
	tWord value;
	memcpy(&value, &RAM[regValue[SP]], WORD_SIZE);
	regValue[cmd.left.type] = value;
	regValue[SP] += WORD_SIZE;
}

void _print (tCommandWithOperands cmd)
{
    printf("print: %d\n", getOperandValue(cmd.left));
}

void _input (tCommandWithOperands cmd)
{
	scanf("%hd", &regValue[cmd.left.type]);
}

void _hlt (tCommandWithOperands cmd)
{
    processing_halted_flag = 1;
}

