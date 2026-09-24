// C header file for x86-64 assembly text in AT&T syntax.

#ifndef TXT_X86_64_H
#define TXT_X86_64_H

#include "arch/x86_64/asm.h"

// AT&T syntax writer
void Txt_x86_64_Att_WriteOperand(File_Stream *out, const Asm_x86_64_Operand *op);
void Txt_x86_64_Att_WriteInstr(File_Stream *out, const Asm_x86_64_Item *item);
void Txt_x86_64_Att_Write(File_Stream *out);

// Name-to-value lookups
char Txt_x86_64_Att_WidthSuffix(Asm_x86_64_Width width);
int Txt_x86_64_Att_ExtendOp(const char *mnem, Asm_x86_64_Width *width);
int Txt_x86_64_RegByName(const char *name, Asm_x86_64_Width *width);
int Txt_x86_64_OpByName(const char *name);

// Text scanning
int Txt_x86_64_Att_IsNameStart(char c);
int Txt_x86_64_Att_IsNameChar(char c);
int Txt_x86_64_Att_IsLabelStart(char c);
int Txt_x86_64_Att_IsTarget(const char *text);
int Txt_x86_64_Att_IsAddress(const char *text);
const char *Txt_x86_64_Att_ScanReg(const char *p);
const char *Txt_x86_64_Att_ScanNumber(const char *p, long *out);

// AT&T syntax parser
int  Txt_x86_64_Att_ParseOperand(const char *text, Asm_x86_64_Operand *op);
void Txt_x86_64_Att_EmitInts(const char *args, int width);
int  Txt_x86_64_Att_EmitAddress(const char *text, int width);
void Txt_x86_64_Att_EmitString(const char *args, int terminate);
void Txt_x86_64_Att_ParseInstr(const char *line);
void Txt_x86_64_Att_ParseDirective(const char *line);
void Txt_x86_64_Att_ParseLine(char *line);
void Txt_x86_64_Att_Parse(const char *text);

#endif // TXT_X86_64_H
