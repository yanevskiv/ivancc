#ifndef ABI_X86_64_H
#define ABI_X86_64_H

#include "ast/ast.h"

// Bytes in one eightbyte, the unit the SysV ABI classifies an argument in.
#define ABI_X86_64_EIGHTBYTE 8

// Largest aggregate the ABI passes in registers; anything wider goes in memory.
#define ABI_X86_64_MAX_REG_SIZE 16

// Byte offsets of the fields in the SysV va_list record the parser builds.
typedef enum Abi_x86_64_VaField Abi_x86_64_VaField;
enum Abi_x86_64_VaField {
    ABI_X86_64_VA_GP_OFFSET = 0,  // bytes of the register save area already read
    ABI_X86_64_VA_FP_OFFSET = 4,  // the same for SSE registers, which none reach yet
    ABI_X86_64_VA_OVERFLOW  = 8,  // next argument above the return address
    ABI_X86_64_VA_REG_SAVE  = 16  // start of the spilled argument registers
};

// The class the SysV ABI gives one eightbyte of an argument.
typedef enum Abi_x86_64_Class Abi_x86_64_Class;
enum Abi_x86_64_Class {
    ABI_X86_64_CLASS_INTEGER, // a general-purpose register carries it
    ABI_X86_64_CLASS_SSE,     // an SSE register carries it; no type reaches this yet
    ABI_X86_64_CLASS_MEMORY   // the stack carries it, or a hidden pointer returns it
};

// Classification
Abi_x86_64_Class Abi_x86_64_Classify(const Ast_Type *type);
int              Abi_x86_64_Eightbytes(const Ast_Type *type);
int              Abi_x86_64_InMemory(const Ast_Type *type);
int              Abi_x86_64_ReturnsInMemory(const Ast_Type *type);

#endif // ABI_X86_64_H
