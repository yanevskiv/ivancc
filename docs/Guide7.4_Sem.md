## Sem

The semantic pass resolves each member access against the type of its left operand. It also draws the line around what this stage supports. Three of its checks turn silently wrong code into a diagnostic, which matters more with aggregates than in any earlier stage.

### Member resolution

A member access arrives from the grammar carrying a name and nothing else. Resolution attaches the member that the name refers to. The member's type then becomes the type of the whole expression.

The pass walks each expression from the bottom up. The left operand therefore already carries a type by the time the member node is reached. `Sem_IsAggregate()` tests that type before anything else happens.

Three distinct failures are possible, and each gets a message of its own. The operand may not be an aggregate, the aggregate may have been declared but never defined, or it may be defined without the named member.

Collapsing them into one message costs the reader the diagnosis. A complaint about an unknown member is misleading when the type was never defined in this translation unit. The `at_complete` test is what separates those two cases.

`Ast_FindMember()` returns the member and `an_member` keeps it for the code generator. `an_type` takes the member's type, so everything above this node sees the member rather than the aggregate. `p->x + 1` then type-checks as an `int` addition.

```c
case AST_NODE_KIND_MEMBER: {
    Ast_Type *type = node->an_lhs->an_type;
    if (! Sem_IsAggregate(type)) {
        Log_ShowErrorAt(node->an_line, "request for member '%s' in something that is not a struct or union", node->an_memname);
    }
    if (! type->at_complete) {
        Log_ShowErrorAt(node->an_line, "'%s' is an incomplete type", Sem_TypeName(type));
    }
    node->an_member = Ast_FindMember(type, node->an_memname);
    if (! node->an_member) {
        Log_ShowErrorAt(node->an_line, "no member named '%s' in '%s'", node->an_memname, Sem_TypeName(type));
    }
    node->an_type = node->an_member->am_type;
} break;
```

### Members as lvalues

An lvalue is an expression that names an object. `Sem_IsLvalue()` is the test, and it already recognises a variable and a dereference. A member access names an object too.

Two things consult that test. Assignment rejects a left operand that is not an lvalue, and the address-of operator rejects an operand whose address cannot be taken. `p.x = 1` and `&p.x` both need the member kind listed.

The test matters in both directions. Omitting the member kind rejects correct code, which any test catches at once. A test that is too permissive accepts `1 = p.x`, which nothing catches unless a test is written for it.

```c
// True if node names an object, so it can be assigned to or have its address taken.
int Sem_IsLvalue(const Ast_Node *node)
{
    return node->an_kind == AST_NODE_KIND_VAR || node->an_kind == AST_NODE_KIND_DEREF
        || node->an_kind == AST_NODE_KIND_MEMBER;
}
```

### Aggregate assignment

Assigning one whole structure to another is the only operation on a complete aggregate that this stage supports. `q = p` copies every member, including the padding between them. The copy itself belongs to the code generator.

The semantic pass decides whether the assignment is allowed at all. Two objects of the same aggregate type may be assigned. Two objects of different types may not, whatever their members happen to look like.

The test is type identity rather than shape. Two structures with identical members are different types, and C says so. The check is therefore a pointer comparison rather than a walk over two member lists.

Identity works because every reference to a tag resolves to one type object. `Par_ReferenceAggregate()` reuses an existing binding rather than creating a second type. A structure defined twice across two translation units would defeat this, which is a linker's concern rather than this pass's.

```c
/* Identity, not shape: two structs with the same members stay distinct. */
if (Sem_IsAggregate(node->an_lhs->an_type) && node->an_lhs->an_type != node->an_rhs->an_type) {
    Log_ShowErrorAt(node->an_line, "cannot assign a value of a different struct or union type");
}
```

### By-value arguments

Passing an aggregate by value requires the SysV ABI's argument classification algorithm. That algorithm decides which members travel in registers and which on the stack. This stage does not implement it.

The code generator will not refuse the case on its own. It leaves an aggregate as an address, so it passes that address where the callee expects a value. The call compiles and runs.

The resulting program is quietly wrong. The callee reads its parameter as a structure sitting at an address it was never given. The failure surfaces somewhere else entirely, often as a corrupted value in an unrelated variable.

`Sem_CheckByValue()` rejects the case instead. It walks `an_args` at every call and tests each argument with `Sem_IsAggregate()`. The message names passing the address as the workaround, so the reader is not left guessing.

```c
// Reject by-value aggregate arguments, which need the ABI's classification.
void Sem_CheckByValue(Ast_Node *node)
{
    for (Ast_Node *arg = node->an_args; arg; arg = arg->an_next) {
        if (Sem_IsAggregate(arg->an_type)) {
            Log_ShowErrorAt(node->an_line, "passing '%s' by value is not supported yet; pass its address", Sem_TypeName(arg->an_type));
        }
    }
}
```

### By-value returns

Returning an aggregate by value needs the same classification algorithm as passing one. The check therefore belongs beside the argument check. It sits in the `AST_NODE_KIND_RETURN` case rather than in a function of its own.

The generator would return the address of a local. That local dies with the frame it sits in. The caller receives a pointer into memory that the next call is free to overwrite.

This failure is worse than a mangled argument. It looks correct until the caller uses the value, and what it produces depends on whatever runs next. A bug that reproduces inconsistently costs far more than a diagnostic at the `return`.

The `an_lhs` test comes first, because `return;` with no value is legal in a `void` function. Only a returned expression is examined. `Sem_IsAggregate()` then makes the same decision it makes for an argument.

```c
case AST_NODE_KIND_RETURN: {
    if (node->an_lhs && Sem_IsAggregate(node->an_lhs->an_type)) {
        Log_ShowErrorAt(node->an_line, "returning '%s' by value is not supported yet", Sem_TypeName(node->an_lhs->an_type));
    }
} break;
```

### Incomplete type dereference

The `AST_NODE_KIND_DEREF` case types a `*p` expression. Its result type is whatever `p` points at. Three things can make that invalid, and all three are checked here.

`Sem_IsPointer()` rejects an operand that is not a pointer at all. A pointer to `void` is rejected next, since `void` has no size to load. Both of those checks predate this stage.

The `at_complete` test is the addition. A pointer to an incomplete type cannot be dereferenced, because the type has no size and no members. The check costs one flag read on an expression that is already being typed.

Catching it here is what makes the message useful. Without the check, the complaint would come from the member lookup or from the code generator. Both would point at a symptom several steps away from the missing definition.

```c
case AST_NODE_KIND_DEREF: {
    if (! Sem_IsPointer(node->an_lhs->an_type)) {
        Log_ShowErrorAt(node->an_line, "indirection requires a pointer operand");
    }
    if (node->an_lhs->an_type->at_base->at_kind == AST_TYPE_KIND_VOID) {
        Log_ShowErrorAt(node->an_line, "cannot dereference a pointer to void");
    }
    if (! node->an_lhs->an_type->at_base->at_complete) {
        Log_ShowErrorAt(node->an_line, "cannot dereference a pointer to an incomplete type");
    }
    node->an_type = node->an_lhs->an_type->at_base;
} break;
```
