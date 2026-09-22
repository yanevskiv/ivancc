## Gen (x86_64)

The generator can move scalar values between registers and addresses, but aggregates need address-preserving member access, whole-object copies, and zeroed initialization. It leaves aggregate expressions in memory, adds member offsets, and emits byte copies and clears for the operations this stage supports. Passing aggregates across calls remains outside this part, so the later ABI work can replace these restrictions.

### Extend: `Gen_x86_64_EmitLoad()`

`Gen_x86_64_EmitLoad()` loads the value at the address in `%rax`, which every scalar expression ends with. The function returns without emitting anything when the type is an array or an aggregate. Only a structure wider than a register produces a wrong answer, so a small test passes.

```c
// Load the value at the address in %rax, unless the type lives as an address.
void Gen_x86_64_EmitLoad(const Ast_Type *type)
{
    if (type->at_kind == AST_TYPE_KIND_ARRAY || Sem_IsAggregate(type)) {
        return;                           /* an aggregate stays an address */
    }
    Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RAX, 0, ASM_X86_64_REG_RAX, Gen_x86_64_TypeWidth(type));
}
```

### Extend: `Gen_x86_64_EmitAddr()`

`Gen_x86_64_EmitAddr()` computes the address of an lvalue, which a variable and a dereference already reach. A case for the member node recurses for the operand's address and adds `am_offset` to it. Chaining then costs one add per step, so `a.b.c` emits an address and two immediate adds.

```c
case AST_NODE_KIND_MEMBER: {
    Gen_x86_64_EmitAddr(node->an_lhs);
    if (node->an_member->am_offset) { /* a first member costs nothing */
        Asm_x86_64_EmitAddImm(node->an_member->am_offset, ASM_X86_64_REG_RAX);
    }
} break;
```

### Extend: `Gen_x86_64_EmitExpr()`

Evaluating an expression for its value is separate from evaluating it for its address, and some node kinds do both. The member kind joins the variable and dereference labels on that shared arm. `&p.x` needs nothing new either, because the address is what the arm produced before any load.

```c
// Emit code for an expression, leaving its result in %rax.
void Gen_x86_64_EmitExpr(Ast_Node *node)
{
    switch (node->an_kind) {

        case AST_NODE_KIND_VAR:
        case AST_NODE_KIND_DEREF:
        case AST_NODE_KIND_MEMBER: {
            Gen_x86_64_EmitAddr(node);
            Gen_x86_64_EmitLoad(node->an_type); /* a no-op for an aggregate */
        } break;

    }
}
```

### Add: `Gen_x86_64_EmitCopy()`

Assigning one whole structure to another moves its bytes from one address to another. `Gen_x86_64_EmitCopy()` emits the move inline, in eight-, four- and one-byte steps. `%rsi` and `%rcx` are used unsaved, which holds only while the generator carries values between subexpressions in `%rax` alone.

```c
// Copy size bytes from the address in %rax to the address in %rdi, leaving the
// destination in %rax so that an assignment yields the object it assigned to.
void Gen_x86_64_EmitCopy(int size)
{
    int off = 0;

    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RSI);
    while (size - off >= GEN_X86_64_COPY_QUAD) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RSI, off, ASM_X86_64_REG_RCX, ASM_X86_64_WIDTH_64);
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_64);
        off += GEN_X86_64_COPY_QUAD;
    }
    while (size - off >= GEN_X86_64_COPY_LONG) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RSI, off, ASM_X86_64_REG_RCX, ASM_X86_64_WIDTH_32);
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_32);
        off += GEN_X86_64_COPY_LONG;
    }
    while (off < size) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RSI, off, ASM_X86_64_REG_RCX, ASM_X86_64_WIDTH_8);
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_8);
        off++;
    }
    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RDI, ASM_X86_64_REG_RAX);
}
```

### Add: `Gen_x86_64_EmitZero()`

C says an initializer's omissions are zero, and for an aggregate those omissions can be anywhere inside it. `Gen_x86_64_EmitZero()` writes zero across a run of bytes at the address in `%rdi`. Nothing is returned in `%rax`, because the statement this serves has no value.

```c
// Write size zero bytes at the address in %rdi.
void Gen_x86_64_EmitZero(int size)
{
    int off = 0;

    Asm_x86_64_EmitMovImm(0, ASM_X86_64_REG_RCX);
    while (size - off >= GEN_X86_64_COPY_QUAD) {
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_64);
        off += GEN_X86_64_COPY_QUAD;
    }
    while (size - off >= GEN_X86_64_COPY_LONG) {
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_32);
        off += GEN_X86_64_COPY_LONG;
    }
    while (off < size) {
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RCX, ASM_X86_64_REG_RDI, off, ASM_X86_64_WIDTH_8);
        off++;
    }
}
```

### Extend: `case AST_NODE_KIND_ASSIGN`

An assignment reaches the generator with its destination address on the stack and its source in `%rax`. The arm branches on `Sem_IsAggregate()` and copies bytes when the answer is yes. The copy therefore needs no preparation, and the scalar path is unchanged from earlier stages.

```c
case AST_NODE_KIND_ASSIGN: {

    Gen_x86_64_EmitPop(ASM_X86_64_REG_RDI);  /* the destination address */
    if (Sem_IsAggregate(node->an_type)) {
        Gen_x86_64_EmitCopy(node->an_type->at_size);
    } else {
        Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI, 0, Gen_x86_64_TypeWidth(node->an_type));
    }
} break;
```

### Extend: `case AST_NODE_KIND_ZERO`

Local initialization emits a statement that clears an object before anything is written to it. A case of its own emits the address, moves it to `%rdi` and calls the clear. An array's element type says nothing about the total, and a structure's size includes padding its members do not account for.

```c
case AST_NODE_KIND_ZERO: {
    Gen_x86_64_EmitAddr(node->an_lhs);
    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI);
    Gen_x86_64_EmitZero((int) node->an_val);
} break;
```

### Modify: `Gen_x86_64_EmitGlobal()`

A global with an initializer needs an image of its bytes at compile time, which the assembler emits into `.data`. Each flattened entry now writes itself at the byte offset it already carries, sized by its own slot's type. The entries may arrive in any order for the same reason, which is what `int sparse[6] = {[4] = 40, [1] = 10};` relies on.

```c
// Emit one global: its bytes in .data when it has an initializer, or the space
// it asks for in .bss when it is zeroed.
void Gen_x86_64_EmitGlobal(Ast_Var *var)
{
    unsigned char *bytes = calloc(size ? size : 1, 1);

    /* Each entry already knows its byte offset and its own slot's type. */
    for (Ast_Node *item = var->av_init; item; item = item->an_next) {
        Gen_x86_64_EmitConstant(bytes, item->an_type->at_size, (int) item->an_val, item->an_lhs, var);
    }

}
```
