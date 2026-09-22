## Gen (x86_64)

The back end needs less than might be expected. `lea`, `add`, and `mov` already exist for scalars and arrays, and aggregates need no instruction beyond them. What changes is when a value is loaded from an address and when it stays an address.

### Aggregates as addresses

No register holds a structure. Anything beyond eight bytes will not fit in one, and this generator cannot split an object across several. Every use of an aggregate therefore wants its address.

Arrays already work this way, which is why aggregates join the existing rule rather than getting one of their own. Only the set of covered types changes, so the decision stays in one function.

`Gen_x86_64_EmitLoad()` is where the rule lives. The test covers `AST_TYPE_KIND_ARRAY` and anything `Sem_IsAggregate()` accepts, returning before it emits, so the address the caller left in `%rax` stays there.

Getting this wrong loads the first eight bytes and treats them as the whole object. The result compiles and runs, and anything that fits in a register happens to work. Only a larger structure fails.

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

A member access needs the address of the member rather than of the aggregate. That address is the aggregate's address plus the member's offset, which the layout pass already computed.

`Gen_x86_64_EmitAddr()` recurses into `an_lhs` for the aggregate's address. The recursion asks for an address rather than a value, so no load falls between the two steps.

`Asm_x86_64_EmitAddImm()` then adds `am_offset`. The offset is a compile-time constant, so the access costs one immediate add. For example, `a.b.c` emits two such adds and no loads at all.

The test on `am_offset` skips the add when the offset is zero. A first member therefore costs no instruction, so `p->val` on `struct Node` emits exactly what `*p` would emit.

```c
case AST_NODE_KIND_MEMBER: {
    Gen_x86_64_EmitAddr(node->an_lhs);
    if (node->an_member->am_offset) { /* a first member costs nothing */
        Asm_x86_64_EmitAddImm(node->an_member->am_offset, ASM_X86_64_REG_RAX);
    }
} break;
```

### Member evaluation

Evaluating an expression for its value is separate from evaluating it for its address. A member access does both, as a variable and a dereference already do, so no new case is needed.

`AST_NODE_KIND_MEMBER` joins `AST_NODE_KIND_VAR` and `AST_NODE_KIND_DEREF` on one case. `Gen_x86_64_EmitAddr()` produces the address and `Gen_x86_64_EmitLoad()` decides whether to load, so the label is the whole change.

Everything else follows from that pairing. For example, `a.b.c` nests because each step takes the address of its operand, and `&p.x` needs nothing new because that address is what the case computed.

```c
case AST_NODE_KIND_VAR:
case AST_NODE_KIND_DEREF:
case AST_NODE_KIND_MEMBER: {
    Gen_x86_64_EmitAddr(node);
    Gen_x86_64_EmitLoad(node->an_type); /* a no-op for an aggregate */
} break;
```

### Byte copying

Assigning a whole aggregate copies its bytes from one address to another. `Gen_x86_64_EmitCopy()` emits that copy inline, with the source address in `%rax` and the destination in `%rdi`.

Emitting the moves inline keeps the compiler independent of `memcpy`, which does not yet exist for this target. A call would also need its argument registers loaded, which this generator does not manage.

The opening `Asm_x86_64_EmitMovRR()` moves the source out of `%rax` and into `%rsi`. Every subexpression returns in `%rax`, so the source cannot stay there while the loops run.

The first loop runs while at least `GEN_X86_64_COPY_QUAD` bytes remain. Each iteration emits a load from `%rsi` into `%rcx` and a store into `%rdi`, both at `ASM_X86_64_WIDTH_64`.

The second loop repeats that pattern at `ASM_X86_64_WIDTH_32`. It covers only what the first loop could not, so it emits at most one pair of instructions.

The third loop finishes the tail at `ASM_X86_64_WIDTH_8`. It runs at most three times, since anything wider was taken already, so a 13-byte structure copies in three pairs.

The closing `Asm_x86_64_EmitMovRR()` moves `%rdi` into `%rax`. An assignment is an expression whose value is the object assigned to, so `a = b = c` depends on that register.

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

Clearing an object writes zero across a run of bytes. `Gen_x86_64_EmitZero()` emits that fill, taking the destination in `%rdi`, and serves the zero statement that local initialization emits.

`Asm_x86_64_EmitMovImm()` loads zero into `%rcx` once, before any loop runs. No load is then needed inside the loops, which is the one structural difference from the copy.

The three loops mirror the copy at eight, four, and one byte. Nothing is returned in `%rax`, because the statement this implements has no value of its own.

The widths matter for output size rather than for speed. A 64-byte object clears in eight stores rather than sixty-four, and this compiler assembles the text it writes.

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

An assignment reaches the generator with its destination address on the stack and its source in `%rax`. This is the only place in the expression generator that has to tell an aggregate from a scalar.

`Gen_x86_64_EmitPop()` recovers the destination into `%rdi`. Both operands are addresses at this point, since `Gen_x86_64_EmitLoad()` declined to load either side, so the copy needs no preparation.

`Sem_IsAggregate()` then selects between the two paths. An aggregate goes to `Gen_x86_64_EmitCopy()` with its type's size, and everything else stores from a register exactly as before.

```c
Gen_x86_64_EmitPop(ASM_X86_64_REG_RDI);  /* the destination address */
if (Sem_IsAggregate(node->an_type)) {
    Gen_x86_64_EmitCopy(node->an_type->at_size);
} else {
    Asm_x86_64_EmitMovStore(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI, 0, Gen_x86_64_TypeWidth(node->an_type));
}
```

### Object zeroing

The zero statement clears an object before its initializer writes to it. It is the one statement kind in the tree with no expression behind it, which is why it needs a case of its own.

`Gen_x86_64_EmitAddr()` leaves the address in `%rax`, so `Asm_x86_64_EmitMovRR()` moves it into `%rdi` where `Gen_x86_64_EmitZero()` expects it. The size comes straight from `an_val`.

Taking the size from the node rather than from a type is what makes the statement general. The object may be an array, whose element type says nothing about the total to clear.

```c
case AST_NODE_KIND_ZERO: {
    Gen_x86_64_EmitAddr(node->an_lhs);
    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RDI);
    Gen_x86_64_EmitZero((int) node->an_val);
} break;
```

### Global initializers

A global with an initializer needs an image of its bytes at compile time. Each flattened entry contributes one value at one offset, and the image is what the assembler emits into `.data`.

`calloc()` zeroes the image as it allocates it, so the gaps a designator leaves need no further attention. `struct Rec rec = {.n = 5};` writes four bytes and leaves the rest alone.

The loop calls `Gen_x86_64_EmitConstant()` once per entry. The width comes from the slot's own type rather than the object's, and the offset from `an_val`, already computed in bytes.

Any code that multiplies an element index by an element size is replaced here. That arithmetic only worked for an array of scalars, and it cannot express a structure with members of different sizes.

```c
unsigned char *bytes = calloc(size ? size : 1, 1);

/* Each entry already knows its byte offset and its own slot's type. */
for (Ast_Node *item = var->av_init; item; item = item->an_next) {
    Gen_x86_64_EmitConstant(bytes, item->an_type->at_size, (int) item->an_val, item->an_lhs, var);
}
```
