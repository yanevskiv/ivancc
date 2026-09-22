#ifndef ABI_X86_64_H
#define ABI_X86_64_H

#include "ast/ast.h"

// Bytes in one eightbyte, the unit the SysV ABI classifies an argument in.
#define ABI_X86_64_EIGHTBYTE 8

// Largest aggregate the ABI passes in registers; anything wider goes in memory.
#define ABI_X86_64_MAX_REG_SIZE 16

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
