## Gen (x86_64)

The generator moved one eightbyte per argument and returned one in `%rax`, which cannot express an aggregate wanting two registers or the caller's buffer. The call sequence is rebuilt around the classification, and the prologue, the epilogue and the frame layout follow it. The caller and the callee derive their allocation from different code, so an ABI change has to be made in both.

### Add: `Gen_x86_64_ArgRegBase()`

Where one argument lands depends on every argument ahead of it, and one that does not fit goes to the stack while a later smaller one may still fit. `Gen_x86_64_ArgRegBase()` replays that allocation and stops at the argument asked about, so every pass reaches the same answer. Replaying rather than recording avoids a parallel array two passes could disagree about, at a cost a handful of arguments never notices.

```c
// Return the first argument register the argument at index takes, or -1 for the stack.
int Gen_x86_64_ArgRegBase(Ast_Node *args, int index, int nHidden)
{
    int used = nHidden;
    int i = 0;

    for (Ast_Node *arg = args; arg; arg = arg->an_next, i++) {
        int want = Abi_x86_64_Eightbytes(arg->an_type);
        if (Abi_x86_64_InMemory(arg->an_type) || used + want > MAX_REG_ARGS) {
            if (i == index) {
                return -1;             /* this one goes on the stack */
            }
            continue;                  /* and consumes no register */
        }
        if (i == index) {
            return used;
        }
        used += want;
    }
    return -1;
}
```

### Add: `Gen_x86_64_CallStackSlots()`

The stack must be 16-byte aligned when the `call` executes, and counting arguments no longer gives the slot count once one aggregate occupies three. `Gen_x86_64_CallStackSlots()` asks `Gen_x86_64_ArgRegBase()` about each argument and sums the eightbytes of those it places in memory. Deriving the count from the same function as the pushes is what stops a separate counting rule from drifting the day the ABI grows a case.

```c
// Count the eightbytes a call leaves on the stack for its memory arguments.
int Gen_x86_64_CallStackSlots(Ast_Node *args, int nHidden)
{
    int slots = 0;
    int i = 0;

    for (Ast_Node *arg = args; arg; arg = arg->an_next, i++) {
        if (Gen_x86_64_ArgRegBase(args, i, nHidden) < 0) {
            slots += Abi_x86_64_Eightbytes(arg->an_type);
        }
    }
    return slots;
}
```

### Add: `Gen_x86_64_PushArg()`

A scalar evaluates to its value while an aggregate evaluates to an address, whose eightbytes must be read out before reaching the stack. `Gen_x86_64_PushArg()` loads each through `%rcx` and pushes it counting down, so the lowest ends on top where the pops expect it. Reversing that loop puts the halves of a structure in the wrong registers, which no one-eightbyte test would catch.

```c
// Evaluate one argument and push its eightbytes, lowest ending on top.
void Gen_x86_64_PushArg(Ast_Node *arg)
{
    Gen_x86_64_EmitExpr(arg);

    if (! Sem_IsAggregate(arg->an_type)) {
        Gen_x86_64_EmitPush();
        return;
    }
    for (int k = Abi_x86_64_Eightbytes(arg->an_type) - 1; k >= 0; k--) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RAX, k * WORD_SIZE, ASM_X86_64_REG_RCX, ASM_X86_64_WIDTH_64);
        Asm_x86_64_EmitPush(ASM_X86_64_REG_RCX);
        Gen_x86_64_Depth++;
    }
}
```

### Add: `Gen_x86_64_CallPushStack()`

A single push pass interleaves stack and register arguments, and popping the register ones would leave holes between the others. `Gen_x86_64_CallPushStack()` pushes only the memory arguments, last one first, so the register pass can build above them and unwind completely. The split changes the order arguments are evaluated in, which C leaves unspecified, and it is the same split a variadic call will need later.

```c
// Push the arguments the ABI places on the stack, last one first.
void Gen_x86_64_CallPushStack(Ast_Node *args, Ast_Node *arg, int index, int nHidden)
{
    if (! arg) {
        return;
    }
    Gen_x86_64_CallPushStack(args, arg->an_next, index + 1, nHidden);  /* last first */
    if (Gen_x86_64_ArgRegBase(args, index, nHidden) < 0) {
        Gen_x86_64_PushArg(arg);
    }
}
```

### Add: `Gen_x86_64_CallPopReg()`

The register pass pushed its arguments last one first, so popping them has to run in argument order rather than in push order. `Gen_x86_64_CallPopReg()` walks the list forwards and pops one register per eightbyte, starting at the base the allocation returned for that argument. Nothing here re-derives the allocation, so a change to the rule cannot desynchronise the push pass from the pop pass.

```c
// Pop the pushed register arguments into the registers the ABI assigns them.
void Gen_x86_64_CallPopReg(Ast_Node *args, int nHidden)
{
    int i = 0;

    for (Ast_Node *arg = args; arg; arg = arg->an_next, i++) {
        int base = Gen_x86_64_ArgRegBase(args, i, nHidden);
        if (base < 0) {
            continue;
        }
        for (int k = 0; k < Abi_x86_64_Eightbytes(arg->an_type); k++) {
            Gen_x86_64_EmitPop(Gen_x86_64_ArgReg[base + k]);
        }
    }
}
```

### Add: `Gen_x86_64_EmitCall()`

A call was a count, a push and a pop held inline, which an aggregate argument or return makes long enough to get wrong. `Gen_x86_64_EmitCall()` runs the whole sequence, loading the hidden pointer after the pops because `%rdi` would otherwise be popped over. Every future ABI change lands in this one function, which is where a variadic call's vector count will go in stage twelve.

```c
// Emit a call, leaving its result in %rax, or an aggregate's address there.
void Gen_x86_64_EmitCall(Ast_Node *node)
{
    int nHidden = Abi_x86_64_ReturnsInMemory(node->an_type) ? 1 : 0;
    int nStack  = Gen_x86_64_CallStackSlots(node->an_args, nHidden);

    Gen_x86_64_CallPushStack(node->an_args, node->an_args, 0, nHidden);
    Gen_x86_64_CallPushReg(node->an_args, node->an_args, 0, nHidden);
    Gen_x86_64_CallPopReg(node->an_args, nHidden);

    if (nHidden) {
        Asm_x86_64_EmitLea(ASM_X86_64_REG_RBP, node->an_tmp, ASM_X86_64_REG_RDI);
    }
    Asm_x86_64_EmitCall(node->an_funcname);

}
```

### Add: `Gen_x86_64_AssignCallTemps()`

A call returning an aggregate needs a frame slot, and one slot per function would let the inner call in `sum(big(10))` overwrite the outer one's buffer. `Gen_x86_64_AssignCallTemps()` walks the body before the locals are placed and gives every such call its own offset. The walk skips the case lists, whose nodes the statement list already reaches, and its recursion would only matter on a far larger function.

```c
// Give every call that returns an aggregate a frame slot to land the result in.
void Gen_x86_64_AssignCallTemps(Ast_Node *node, int *offset)
{
    if (! node) {
        return;
    }
    if (node->an_kind == AST_NODE_KIND_CALL && Sem_IsAggregate(node->an_type)) {
        *offset = Gen_x86_64_AlignTo(*offset + node->an_type->at_size, node->an_type->at_align);
        node->an_tmp = -*offset;
    }

}
```

### Add: `Gen_x86_64_EmitReturnValue()`

A `return` left its result in `%rax`, which an aggregate cannot do because it arrives as an address rather than a value. `Gen_x86_64_EmitReturnValue()` copies the object into the caller's buffer for a memory return, and loads its eightbytes into `%rax` and `%rdx` for a register one. The address moves to `%rcx` first, since the low eightbyte overwrites `%rax` and a one-eightbyte structure would hide that bug entirely.

```c
// Turn the value of a return expression into what the ABI returns.
void Gen_x86_64_EmitReturnValue(Ast_Node *node)
{
    if (! Sem_IsAggregate(node->an_type)) {
        return;
    }
    if (Abi_x86_64_ReturnsInMemory(node->an_type)) {
        Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RBP, Gen_x86_64_RetPtrOffset, ASM_X86_64_REG_RDI, ASM_X86_64_WIDTH_64);
        Gen_x86_64_EmitCopy(node->an_type->at_size);
        return;
    }
    Asm_x86_64_EmitMovRR(ASM_X86_64_REG_RAX, ASM_X86_64_REG_RCX);  /* before %rax is reused */

    Asm_x86_64_EmitMovLoad(ASM_X86_64_REG_RCX, 0, ASM_X86_64_REG_RAX, ASM_X86_64_WIDTH_64);
}
```

### Add: `Gen_x86_64_EmitParam()`

The prologue spilled one register per parameter indexed by position, which breaks the moment an aggregate parameter advances the index by two. `Gen_x86_64_EmitParam()` takes `reg` and `stack` by pointer and steps each by whatever that parameter actually consumed. The register test repeats the caller's rule rather than calling `Gen_x86_64_ArgRegBase()`, so a change to one has to be made to the other.

```c
// Spill one incoming parameter into its frame slot.
void Gen_x86_64_EmitParam(Ast_Var *param, int *reg, int *stack)
{
    int slots = Abi_x86_64_Eightbytes(param->av_type);
    int inReg = ! Abi_x86_64_InMemory(param->av_type) && *reg + slots <= MAX_REG_ARGS;

    for (int k = 0; k < slots; k++) {
        if (inReg) {
            Asm_x86_64_EmitMovStore(Gen_x86_64_ArgReg[(*reg)++], ASM_X86_64_REG_RBP, param->av_offset + k * WORD_SIZE, ASM_X86_64_WIDTH_64);
        } else {

        }
    }
}
```

### Extend: `Gen_x86_64_AssignLvarOffsets()`

A function returning in memory receives a pointer in `%rdi`, which the first parameter would otherwise claim before the prologue could save it. The function reserves a slot for that pointer ahead of every local, and places the call temps after them from the same running offset. `Gen_x86_64_RetPtrOffset` is a file-scope variable because only this module reads it, and zero is a sentinel no real slot can collide with.

```c
// Assign each local a stack slot and record the frame size.
void Gen_x86_64_AssignLvarOffsets(Ast_Func *func)
{
    int offset = func->af_variadic ? VA_SAVE_SIZE : 0;

    if (Abi_x86_64_ReturnsInMemory(func->af_ret)) {
        offset += WORD_SIZE;
        Gen_x86_64_RetPtrOffset = -offset;   /* ahead of every local */
    } else {
        Gen_x86_64_RetPtrOffset = 0;
    }

}
```

### Extend: `Gen_x86_64_EmitAddr()`

The switch rejected a call as not an lvalue, which `add(p, q).x` needs accepted because C gives a returned structure temporary lifetime. A case for `AST_NODE_KIND_CALL` emits the call and leaves the frame slot's address behind, falling through to the usual diagnostic for a scalar result. `add(p, q).x = 1` is still accepted, because the lvalue test sees a member node and cannot tell what it hangs off.

```c
// Compute the address of an lvalue into %rax.
void Gen_x86_64_EmitAddr(Ast_Node *node)
{
    switch (node->an_kind) {

        case AST_NODE_KIND_CALL: {
            if (! Sem_IsAggregate(node->an_type)) {
                Log_ShowErrorAt(node->an_line, "codegen: not an lvalue");
            }
            Gen_x86_64_EmitCall(node);
        } break;

    }
}
```
