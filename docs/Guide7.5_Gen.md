## Gen (x86_64)

The back end needs less than might be expected. `lea`, `add`, and `mov` already exist for scalars and arrays, and aggregates need no instruction beyond them. What changes is when a value is loaded from an address and when it stays an address.

### Aggregates as addresses

No register holds a structure. Anything beyond eight bytes will not fit in one, and this generator has no way to split an object across several. Every use of an aggregate therefore wants its address.

Arrays already work this way in the generator. An array expression evaluates to the address of its first element rather than to a value. Aggregates join that existing rule rather than needing one of their own.

`Gen_x86_64_EmitLoad()` is where the rule lives. It returns without emitting anything when the type is an array or an aggregate, leaving the address in `%rax`. Every other type gets the `mov` it always got.

Getting this wrong loads the first eight bytes and treats them as the whole object. The result compiles and runs. Anything larger than a register produces nonsense, while anything smaller happens to work.

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

### Member addressing

A member access needs the address of the member. That address is the address of the aggregate plus the member's offset. The layout pass computed that offset and the semantic pass attached it to the node.

`Gen_x86_64_EmitAddr()` recurses into `an_lhs` to produce the aggregate's address in `%rax`. `Asm_x86_64_EmitAddImm()` then adds `am_offset` to it. Nothing further is required, because the offset is a compile-time constant.

The test on `am_offset` skips the add when the offset is zero. A first member therefore costs no instruction at all. `p->val` on `struct Node` is exactly as cheap as `*p` would be.

Chaining costs one addition per step. `a.b.c` emits the address of `a` followed by two immediate adds. No load falls between them, because neither intermediate is a scalar.

```c
case AST_NODE_KIND_MEMBER: {
    Gen_x86_64_EmitAddr(node->an_lhs);
    if (node->an_member->am_offset) { /* a first member costs nothing */
        Asm_x86_64_EmitAddImm(node->an_member->am_offset, ASM_X86_64_REG_RAX);
    }
} break;
```

### Member evaluation

Evaluating an expression for its value is separate from evaluating it for its address. Some node kinds do both, computing an address and then loading through it. A member access is one of them.

`AST_NODE_KIND_MEMBER` joins `AST_NODE_KIND_VAR` and `AST_NODE_KIND_DEREF` on that shared case. `Gen_x86_64_EmitAddr()` produces the address and `Gen_x86_64_EmitLoad()` decides whether to load through it. Adding the label is the whole of the change.

Everything else follows from that pairing. `a.b.c` nests because each step takes the address of its operand, and `p->q` is a dereference followed by an offset. `&p.x` needs nothing new, since the address is what was computed first.

```c
case AST_NODE_KIND_VAR:
case AST_NODE_KIND_DEREF:
case AST_NODE_KIND_MEMBER: {
    Gen_x86_64_EmitAddr(node);
    Gen_x86_64_EmitLoad(node->an_type); /* a no-op for an aggregate */
} break;
```

### Byte copying

Assigning a whole aggregate copies its bytes from one address to another. `Gen_x86_64_EmitCopy()` emits that copy inline. The source address arrives in `%rax` and the destination in `%rdi`.

Three loops cover the size in eight-, four-, and one-byte moves. Each loop runs while at least that many bytes remain. A 16-byte structure therefore copies in two moves rather than sixteen.

Emitting the moves inline keeps the compiler independent of `memcpy`. The C library for this target does not yet exist. A call would also need the argument registers, which this generator does not manage.

`%rax` holds the destination once the copy finishes. An assignment is an expression whose value is the object assigned to. `a = b = c` depends on that, and so does any assignment used as a condition.

`%rsi` and `%rcx` are used freely here without being saved first. A stack-based generator carries values only in `%rax` between subexpressions. Which registers are free is a property of the generator rather than of the ABI.

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

### Byte clearing

Clearing an object writes zero across a run of bytes. `Gen_x86_64_EmitZero()` emits that fill, taking the destination address in `%rdi`. It exists to serve the zero statement that local initialization emits.

The walk is the same as the copy with one operand instead of two. Zero is loaded into `%rcx` once, then stored in the same three widths. Nothing is returned, because the statement it implements has no value.

The widths matter for output size rather than for speed at this stage. A 64-byte object clears in eight stores rather than sixty-four. The emitted text shrinks by the same factor.

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

### Aggregate assignment

An assignment node reaches the generator with its destination address on the stack and its source in `%rax`. What happens next depends on the type being assigned. This is the only place in the expression generator that has to tell an aggregate from a scalar.

`Gen_x86_64_EmitPop()` recovers the destination address into `%rdi` first. `Sem_IsAggregate()` then selects between the two paths. An aggregate goes to `Gen_x86_64_EmitCopy()` with the size the layout pass assigned its type.

Everything else stores from a register exactly as before this stage. `Gen_x86_64_TypeWidth()` picks the store width from the type. Neither path changed for scalars, so nothing that already worked needs retesting.

```c
Gen_x86_64_EmitPop(ASM_X86_64_REG_RDI);  /* the destination address */
if (Sem_IsAggregate(node->an_type)) {
    Gen_x86_64_EmitCopy(node->an_type->at_size);
} else {
    Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI, 0, Gen_x86_64_TypeWidth(node->an_type));
}
```

### Object zeroing

The zero statement clears an object before its initializer writes to it. It carries an address and a size, and nothing else. It is the one statement kind in the tree with no expression behind it.

`Gen_x86_64_EmitAddr()` produces the object's address from `an_lhs`. `Asm_x86_64_EmitMovRR()` moves it into `%rdi`, where `Gen_x86_64_EmitZero()` expects to find it. The size comes straight from `an_val`.

Taking the size from the node rather than from a type is deliberate. The object being cleared may be an array, whose element type says nothing about the total. A statement kind of its own is what lets the parser settle the size once.

```c
case AST_NODE_KIND_ZERO: {
    Gen_x86_64_EmitAddr(node->an_lhs);
    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI);
    Gen_x86_64_EmitZero((int) node->an_val);
} break;
```

### Global initializers

A global with an initializer needs an image of its bytes at compile time. Each flattened entry contributes one value at one offset. The finished image is what the assembler emits into `.data`.

`calloc()` allocates the image and zeroes it in the same call. The gaps a designator leaves therefore need no further attention. The entries may arrive in any order, since each one carries its own offset.

The loop walks `av_init` and calls `Gen_x86_64_EmitConstant()` once per entry. The width comes from `item->an_type->at_size`, which is the slot's own type. The offset comes from `an_val`, which the flattener already computed in bytes.

Any existing code that multiplies an element index by an element size is replaced here. That arithmetic only ever worked for an array of scalars. A structure whose members have different sizes cannot be expressed by it at all.

```c
unsigned char *bytes = calloc(size ? size : 1, 1);

/* Each entry already knows its byte offset and its own slot's type. */
for (Ast_Node *item = var->av_init; item; item = item->an_next) {
    Gen_x86_64_EmitConstant(bytes, item->an_type->at_size, (int) item->an_val, item->an_lhs, var);
}
```
