// C header file for x86-64 code generation.

#ifndef GEN_X86_64_H
#define GEN_X86_64_H

#include "syntax/ast.h"
#include "arch/x86_64/asm.h"

// Bytes in one eightbyte, the unit the SysV ABI classifies an argument in.
#define GEN_X86_64_SYSV_EIGHTBYTE 8

// Largest aggregate the ABI passes in registers.
#define GEN_X86_64_SYSV_MAX_REG_SIZE 16

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
    GEN_X86_64_SYSV_CLASS_INTEGER, // a general-purpose register carries it
    GEN_X86_64_SYSV_CLASS_SSE,     // an SSE register carries it; no type reaches this yet
    GEN_X86_64_SYSV_CLASS_MEMORY,  // the stack carries it, or a hidden pointer returns it
    GEN_X86_64_SYSV_CLASS_COUNT    // number of classes
};

// An address a global's image holds.
typedef struct Gen_x86_64_Addr Gen_x86_64_Addr;
struct Gen_x86_64_Addr {
    int32_t     ga_offset; // bytes into the image the address occupies
    const char *ga_symbol; // symbol the address is taken from
};

// SysV classification
Gen_x86_64_SysV_Class Gen_x86_64_SysV_Classify(const Ast_Type *type);
int32_t               Gen_x86_64_SysV_Eightbytes(const Ast_Type *type);
bool                  Gen_x86_64_SysV_InMemory(const Ast_Type *type);
bool                  Gen_x86_64_SysV_ReturnsInMemory(const Ast_Type *type);

// SysV calls
void    Gen_x86_64_SysV_EmitReturnValue(Ast_Node *node);
void    Gen_x86_64_SysV_EmitParam(Ast_Var *param, int32_t *reg, int32_t *stack);
int32_t Gen_x86_64_SysV_ArgRegBase(Ast_Node *args, int32_t index, int32_t nHidden);
int32_t Gen_x86_64_SysV_CallStackSlots(Ast_Node *args, int32_t nHidden);
void    Gen_x86_64_SysV_PushArg(Ast_Node *arg);
void    Gen_x86_64_SysV_CallPushStack(Ast_Node *args, Ast_Node *arg, int32_t index, int32_t nHidden);
void    Gen_x86_64_SysV_CallPushReg(Ast_Node *args, Ast_Node *arg, int32_t index, int32_t nHidden);
void    Gen_x86_64_SysV_CallPopReg(Ast_Node *args, int32_t nHidden);
void    Gen_x86_64_SysV_EmitCall(Ast_Node *node);

// SysV variadic arguments
void Gen_x86_64_SysV_EmitVaSaveArea(void);
void Gen_x86_64_SysV_CountNamedArgs(const Ast_Func *func, int32_t *reg, int32_t *stack);
void Gen_x86_64_SysV_EmitVaStart(void);
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

// Expressions, statements and data
void Gen_x86_64_EmitNarrow(const Ast_Type *type);
void Gen_x86_64_EmitDivide(Ast_TypeSign sign, Asm_x86_64_Reg reg);
void Gen_x86_64_EmitShift(Ast_TypeSign sign, Asm_x86_64_Reg reg);
void Gen_x86_64_EmitOpAssign(Ast_NodeKind op, const Ast_Type *type, Ast_Line line);
void Gen_x86_64_EmitExpr(Ast_Node *node);
void Gen_x86_64_EmitStmt(Ast_Node *node);
void Gen_x86_64_AssignCallTemps(Ast_Node *node, int32_t *offset);
void Gen_x86_64_AssignLvarOffsets(Ast_Func *func);
void Gen_x86_64_EmitDataSection(void);
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
