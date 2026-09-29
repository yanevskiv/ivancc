/*
 * C header file for x86-64 code generation.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivancc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivancc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ivancc.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef GEN_X86_64_H
#define GEN_X86_64_H

// Standard headers.
#include <stdlib.h>
#include <string.h>

// Project headers.
#include "util/console/err.h"
#include "util/fp.h"
#include "util/object/elf.h"
#include "lang/ast.h"
#include "lang/sem.h"
#include "arch/x86_64/asm.h"

// Bytes in one eightbyte, the unit the SysV ABI classifies an argument in.
#define GEN_X86_64_SYSV_EIGHTBYTE 8

// Largest aggregate the ABI passes in registers.
#define GEN_X86_64_SYSV_MAX_REG_SIZE 16

// Most eightbytes one value passed in registers occupies.
#define GEN_X86_64_SYSV_MAX_EIGHTBYTES (GEN_X86_64_SYSV_MAX_REG_SIZE / GEN_X86_64_SYSV_EIGHTBYTE)

// The sign bit of a float and of a double.
#define GEN_X86_64_FLOAT_SIGN  0x80000000LL
#define GEN_X86_64_DOUBLE_SIGN ((int64_t) 1 << 63)

// The float bits of 2^63 and 2^64, the edges of the unsigned 64-bit range.
#define GEN_X86_64_FLOAT_TWO_TO_63 0x5F000000
#define GEN_X86_64_FLOAT_TWO_TO_64 0x5F800000

// The shift that brings a 64-bit sign bit down to bit 0 and back.
#define GEN_X86_64_SIGN_SHIFT 63

// Byte offsets of the fields in the SysV va_list record the parser builds.
typedef enum Gen_x86_64_SysV_VaField Gen_x86_64_SysV_VaField;
enum Gen_x86_64_SysV_VaField {
    GEN_X86_64_SYSV_VA_GP_OFFSET = 0,  // bytes of the register save area already read
    GEN_X86_64_SYSV_VA_FP_OFFSET = 4,  // the same for SSE registers
    GEN_X86_64_SYSV_VA_OVERFLOW  = 8,  // next argument above the return address
    GEN_X86_64_SYSV_VA_REG_SAVE  = 16  // start of the spilled argument registers
};

// The class the SysV ABI gives one eightbyte of an argument.
typedef enum Gen_x86_64_SysV_Class Gen_x86_64_SysV_Class;
enum Gen_x86_64_SysV_Class {
    GEN_X86_64_SYSV_CLASS_NONE,    // no member reaches the eightbyte
    GEN_X86_64_SYSV_CLASS_INTEGER, // a general-purpose register carries it
    GEN_X86_64_SYSV_CLASS_SSE,     // an SSE register carries it
    GEN_X86_64_SYSV_CLASS_X87,     // the low eightbyte of a long double
    GEN_X86_64_SYSV_CLASS_X87UP,   // the high eightbyte of a long double
    GEN_X86_64_SYSV_CLASS_MEMORY,  // the stack carries it, or a hidden pointer returns it
    GEN_X86_64_SYSV_CLASS_COUNT    // number of classes
};

// An address a global's image holds.
typedef struct Gen_x86_64_Addr Gen_x86_64_Addr;
struct Gen_x86_64_Addr {
    int32_t     ga_offset; // bytes into the image the address occupies
    const char *ga_symbol; // symbol the address is taken from
};

// The argument registers and stack bytes the values placed so far have taken.
typedef struct Gen_x86_64_SysV_Cursor Gen_x86_64_SysV_Cursor;
struct Gen_x86_64_SysV_Cursor {
    int32_t gc_gpr;   // general-purpose registers taken
    int32_t gc_sse;   // SSE registers taken
    int32_t gc_stack; // bytes of the stack argument area taken
};

// Where the ABI places one argument.
typedef struct Gen_x86_64_SysV_Loc Gen_x86_64_SysV_Loc;
struct Gen_x86_64_SysV_Loc {
    Gen_x86_64_SysV_Class gl_class[GEN_X86_64_SYSV_MAX_EIGHTBYTES];
    int32_t               gl_count; // eightbytes the argument occupies
    int32_t               gl_gpr;   // first general-purpose register it takes
    int32_t               gl_sse;   // first SSE register it takes
    int32_t               gl_stack; // bytes into the stack argument area, or -1
};

// SysV classification
Gen_x86_64_SysV_Class Gen_x86_64_SysV_Merge(Gen_x86_64_SysV_Class a, Gen_x86_64_SysV_Class b);
void                  Gen_x86_64_SysV_ClassifyAt(const Ast_Type *type, int32_t offset, Gen_x86_64_SysV_Class *classes);
void                  Gen_x86_64_SysV_Classify(const Ast_Type *type, Gen_x86_64_SysV_Class *classes);
int32_t               Gen_x86_64_SysV_Eightbytes(const Ast_Type *type);
bool                  Gen_x86_64_SysV_InMemory(const Ast_Type *type);
bool                  Gen_x86_64_SysV_ReturnsInMemory(const Ast_Type *type);
bool                  Gen_x86_64_SysV_ReturnsInX87(const Ast_Type *type);

// SysV argument placement
void                 Gen_x86_64_SysV_StartCursor(Gen_x86_64_SysV_Cursor *cur, const Ast_Type *ret);
void                 Gen_x86_64_SysV_Place(const Ast_Type *type, Gen_x86_64_SysV_Cursor *cur, Gen_x86_64_SysV_Loc *loc);
Gen_x86_64_SysV_Loc *Gen_x86_64_SysV_PlaceArgs(Ast_Node *args, Gen_x86_64_SysV_Cursor *cur);
const Ast_Type      *Gen_x86_64_SysV_PassedType(const Ast_Func *func, const Ast_Var *param);

// SysV calls
void Gen_x86_64_SysV_EmitReturnValue(Ast_Node *node);
void Gen_x86_64_SysV_EmitParam(Ast_Var *param, const Ast_Type *passed, const Gen_x86_64_SysV_Loc *loc);
void Gen_x86_64_SysV_EmitParams(const Ast_Func *func);
void Gen_x86_64_SysV_PushArg(Ast_Node *arg);
void Gen_x86_64_SysV_CallPushStack(Ast_Node *arg, const Gen_x86_64_SysV_Loc *loc, int32_t *end);
void Gen_x86_64_SysV_CallPushReg(Ast_Node *arg, const Gen_x86_64_SysV_Loc *loc);
void Gen_x86_64_SysV_CallPopReg(Ast_Node *args, const Gen_x86_64_SysV_Loc *locs);
void Gen_x86_64_SysV_EmitCallResult(Ast_Node *node);
void Gen_x86_64_SysV_EmitCall(Ast_Node *node);

// SysV variadic arguments
void Gen_x86_64_SysV_EmitVaSaveArea(void);
void Gen_x86_64_SysV_CountNamedArgs(const Ast_Func *func, Gen_x86_64_SysV_Cursor *cur);
void Gen_x86_64_SysV_EmitVaStart(void);
void Gen_x86_64_SysV_EmitVaNext(Gen_x86_64_SysV_VaField field, int32_t limit, int32_t step);
void Gen_x86_64_SysV_EmitVaOverflow(const Ast_Type *type);
void Gen_x86_64_SysV_EmitVaArg(const Ast_Type *type);

// Code emission helpers
int32_t          Gen_x86_64_Count(void);
void             Gen_x86_64_EmitPush(void);
void             Gen_x86_64_EmitPop(Asm_x86_64_Reg reg);
int32_t          Gen_x86_64_AlignTo(int32_t n, int32_t align);
int32_t          Gen_x86_64_SlotSize(const Ast_Type *type);
Asm_x86_64_Width Gen_x86_64_TypeWidth(const Ast_Type *type);
void             Gen_x86_64_EmitAddr(Ast_Node *node);
Ast_TypeSign     Gen_x86_64_Sign(const Ast_Node *node);
void             Gen_x86_64_EmitLoadFrom(Asm_x86_64_Reg base, int32_t disp, Asm_x86_64_Reg dst, const Ast_Type *type);
void             Gen_x86_64_EmitLoad(const Ast_Type *type);
void             Gen_x86_64_EmitCast(const Ast_Type *type);
void             Gen_x86_64_EmitCopy(int32_t size);
void             Gen_x86_64_EmitZero(int32_t size);

// Bitfields
const Ast_Member *Gen_x86_64_Bitfield(const Ast_Node *node);
void              Gen_x86_64_EmitBitfieldLoad(const Ast_Member *member);
void              Gen_x86_64_EmitBitfieldStore(const Ast_Member *member);

// Floating point
bool          Gen_x86_64_ByAddress(const Ast_Type *type);
bool          Gen_x86_64_IsSse(const Ast_Type *type);
bool          Gen_x86_64_IsWideUnsigned(const Ast_Type *type);
Asm_x86_64_Op Gen_x86_64_SseOp(Ast_NodeKind kind, const Ast_Type *type);
Asm_x86_64_Op Gen_x86_64_X87Op(Ast_NodeKind kind);
void          Gen_x86_64_EmitFNum(const Ast_Node *node);
void          Gen_x86_64_EmitX87Result(int32_t tmp);
void          Gen_x86_64_EmitIntToX87(const Ast_Type *from);
void          Gen_x86_64_EmitToX87(const Ast_Type *from);
void          Gen_x86_64_EmitX87ToSse(const Ast_Type *to);
void          Gen_x86_64_EmitX87ToInt(const Ast_Type *to);
void          Gen_x86_64_EmitConvert(const Ast_Type *from, const Ast_Type *to, int32_t tmp);
void          Gen_x86_64_EmitFloatCompare(Ast_NodeKind kind);
void          Gen_x86_64_EmitFloatBinary(Ast_Node *node);
void          Gen_x86_64_EmitFloatNeg(Ast_Node *node);

// Expressions, statements and data
void Gen_x86_64_EmitNarrow(const Ast_Type *type);
void Gen_x86_64_EmitDivide(Ast_TypeSign sign, Asm_x86_64_Reg reg);
void Gen_x86_64_EmitShift(Ast_TypeSign sign, Asm_x86_64_Reg reg);
void Gen_x86_64_EmitOpAssign(Ast_NodeKind op, const Ast_Type *type, Ast_Line line);
void Gen_x86_64_EmitExpr(Ast_Node *node);
void Gen_x86_64_EmitStmt(Ast_Node *node);
bool Gen_x86_64_NeedsTemp(const Ast_Node *node);
void Gen_x86_64_AssignTemps(Ast_Node *node, int32_t *offset);
void Gen_x86_64_AssignLvarOffsets(Ast_Func *func);
void Gen_x86_64_EmitDataSection(void);
void Gen_x86_64_EmitFloatConstant(uint8_t *bytes, const Ast_Node *item, const Ast_Var *var);
void Gen_x86_64_EmitConstant(uint8_t *bytes, const Ast_Node *item, const Ast_Var *var, Gen_x86_64_Addr *addrs, int32_t *naddrs);
void Gen_x86_64_EmitImage(const uint8_t *bytes, int32_t size, const Gen_x86_64_Addr *addrs, int32_t naddrs);
void Gen_x86_64_EmitGlobal(Ast_Var *var);
void Gen_x86_64_EmitGlobals(void);

// Text section
void Gen_x86_64_EmitFunctions(Ast_Func *prog);
void Gen_x86_64_EmitTextSection(Ast_Func *prog);

// Top-level code generation
void Gen_x86_64_BuildProgram(Ast_Func *prog);

#endif // GEN_X86_64_H
