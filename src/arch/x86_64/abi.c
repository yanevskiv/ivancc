#include "syntax/sem.h"
#include "arch/x86_64/abi.h"

// Give a type the class the SysV ABI passes it by.
Abi_x86_64_SysV_Class Abi_x86_64_SysV_Classify(const Ast_Type *type)
{
    if (! Sem_IsAggregate(type)) {
        return ABI_X86_64_SYSV_CLASS_INTEGER;
    }
    if (type->at_size > ABI_X86_64_SYSV_MAX_REG_SIZE) {
        return ABI_X86_64_SYSV_CLASS_MEMORY;
    }
    return ABI_X86_64_SYSV_CLASS_INTEGER;
}

// Return the number of registers or stack slots a type occupies when passed.
int Abi_x86_64_SysV_Eightbytes(const Ast_Type *type)
{
    int size = Sem_IsAggregate(type) ? type->at_size : ABI_X86_64_SYSV_EIGHTBYTE;
    return (size + ABI_X86_64_SYSV_EIGHTBYTE - 1) / ABI_X86_64_SYSV_EIGHTBYTE;
}

// True when an argument of this type is passed on the stack.
int Abi_x86_64_SysV_InMemory(const Ast_Type *type)
{
    return Abi_x86_64_SysV_Classify(type) == ABI_X86_64_SYSV_CLASS_MEMORY;
}

// True when this type is returned through a hidden pointer to the caller's buffer.
int Abi_x86_64_SysV_ReturnsInMemory(const Ast_Type *type)
{
    return type && Sem_IsAggregate(type) && type->at_size > ABI_X86_64_SYSV_MAX_REG_SIZE;
}
