## Abi (x86_64)

The code generator had no way to ask where an aggregate travels, because nothing in the compiler named the classes the SysV ABI assigns. A module of its own answers that in four functions over one enum, which both sides of a call consult so neither can disagree. The classification here is the size rule alone, and stage twelve is where the member walk arrives along with floating point.

### Add: `Abi_x86_64_Class`

The ABI splits an argument into eightbytes and gives each one a class, and nothing in the compiler had a vocabulary for those classes. `Abi_x86_64_Class` names the three the ABI defines, so every decision about where an argument travels reduces to one value this module returns. SSE is unreachable until a floating-point type exists, and declaring it now keeps the enum honest for the merge rule stage twelve needs.

```c
// The class the SysV ABI gives one eightbyte of an argument.
typedef enum Abi_x86_64_Class Abi_x86_64_Class;
enum Abi_x86_64_Class {
    ABI_X86_64_CLASS_INTEGER, // a general-purpose register carries it
    ABI_X86_64_CLASS_SSE,     // an SSE register carries it; no type reaches this yet
    ABI_X86_64_CLASS_MEMORY   // the stack carries it, or a hidden pointer returns it
};
```

### Add: `Abi_x86_64_Classify()`

A class applies to one eightbyte while a caller wants the class of a whole argument, and the two coincide only while nothing mixes an integer and a float inside one. `Abi_x86_64_Classify()` returns MEMORY for an aggregate wider than `ABI_X86_64_MAX_REG_SIZE` and INTEGER for everything else. The real algorithm walks the members and merges their classes, which changes the answer only for a floating-point member or an unaligned one.

```c
// Give a type the class the SysV ABI passes it by.
Abi_x86_64_Class Abi_x86_64_Classify(const Ast_Type *type)
{
    if (! Sem_IsAggregate(type)) {
        return ABI_X86_64_CLASS_INTEGER;
    }
    if (type->at_size > ABI_X86_64_MAX_REG_SIZE) {
        return ABI_X86_64_CLASS_MEMORY;
    }
    return ABI_X86_64_CLASS_INTEGER;
}
```

### Add: `Abi_x86_64_Eightbytes()`

One argument can consume two of the six registers, and a class alone cannot say how many to allocate. `Abi_x86_64_Eightbytes()` rounds an aggregate's size up and answers one for everything else, so both sides step their index by the same amount. A rounded count makes the caller read four bytes past a 12-byte structure, which is what the ABI asks for and what a real compiler emits.

```c
// Return the number of registers or stack slots a type occupies when passed.
int Abi_x86_64_Eightbytes(const Ast_Type *type)
{
    int size = Sem_IsAggregate(type) ? type->at_size : ABI_X86_64_EIGHTBYTE;
    return (size + ABI_X86_64_EIGHTBYTE - 1) / ABI_X86_64_EIGHTBYTE;
}
```

### Add: `Abi_x86_64_ReturnsInMemory()`

A MEMORY result has nowhere to sit, because the caller unwinds the stack immediately after the call returns. `Abi_x86_64_ReturnsInMemory()` identifies that case, which the ABI answers with a hidden pointer to a buffer the caller owns and the callee fills. A null type is tolerated, since `af_ret` is empty for a function no prototype declared and such a call is typed `int` anyway.

```c
// True when this type is returned through a hidden pointer to the caller's buffer.
int Abi_x86_64_ReturnsInMemory(const Ast_Type *type)
{
    return type && Sem_IsAggregate(type) && type->at_size > ABI_X86_64_MAX_REG_SIZE;
}
```
