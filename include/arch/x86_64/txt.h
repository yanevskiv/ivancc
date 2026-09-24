// C header file for x86-64 assembly text in AT&T syntax.

#ifndef TXT_X86_64_H
#define TXT_X86_64_H

#include <stdbool.h>
#include <stddef.h>

#include "util/file.h"
#include "arch/x86_64/asm.h"

// Whether a string directive appends a terminating NUL.
typedef enum Txt_x86_64_Terminate Txt_x86_64_Terminate;
enum Txt_x86_64_Terminate {
    TXT_X86_64_BARE,      // .ascii
    TXT_X86_64_TERMINATED // .string and .asciz
};

// AT&T syntax writer
void Txt_x86_64_Att_WriteOperand(File_Stream *out, const Asm_x86_64_Operand *op);
void Txt_x86_64_Att_WriteInstr(File_Stream *out, const Asm_x86_64_Item *item);
void Txt_x86_64_Att_Write(File_Stream *out);

// Name-to-value lookups
char    Txt_x86_64_Att_WidthSuffix(Asm_x86_64_Width width);
int32_t Txt_x86_64_Att_ExtendOp(const char *mnem, Asm_x86_64_Width *width);
int32_t Txt_x86_64_RegByName(const char *name, Asm_x86_64_Width *width);
int32_t Txt_x86_64_OpByName(const char *name);

// Text scanning
bool Txt_x86_64_Att_IsNameStart(char c);
bool Txt_x86_64_Att_IsNameChar(char c);
bool Txt_x86_64_Att_IsLabelStart(char c);
bool Txt_x86_64_Att_IsTarget(const char *text);
bool Txt_x86_64_Att_IsAddress(const char *text);
const char *Txt_x86_64_Att_ScanReg(const char *p);
const char *Txt_x86_64_Att_ScanNumber(const char *p, int64_t *out);

// AT&T syntax parser
bool Txt_x86_64_Att_ParseOperand(const char *text, Asm_x86_64_Operand *op);
void Txt_x86_64_Att_EmitInts(const char *args, size_t width);
bool Txt_x86_64_Att_EmitAddress(const char *text, size_t width);
void Txt_x86_64_Att_EmitString(const char *args, Txt_x86_64_Terminate terminate);
void Txt_x86_64_Att_ParseInstr(const char *line);
void Txt_x86_64_Att_ParseDirective(const char *line);
void Txt_x86_64_Att_ParseLine(char *line);
void Txt_x86_64_Att_Parse(const char *text);

#endif // TXT_X86_64_H
