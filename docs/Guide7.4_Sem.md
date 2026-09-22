## Sem

The semantic pass resolves each member access against the type of its left operand. It also draws the line around what this stage supports, rejecting what the code generator cannot yet compile correctly. Three of its checks turn silently wrong code into a diagnostic.

### Member resolution

A member access arrives carrying a name and nothing else. Resolution attaches the member that name refers to, and the pass walks bottom-up so the left operand already carries a type here.

Three distinct failures are possible, and each gets a message of its own. The operand may not be an aggregate, the aggregate may never have been defined, or it may lack the named member.

`Sem_IsAggregate()` runs first, on the type of `an_lhs`. Nothing below it would be meaningful on a scalar, so `n.x` where `n` is an `int` has to be rejected here.

The `at_complete` test runs next. A type left incomplete by a bare `struct S;` has an empty member list, so without this test the lookup below would report every member as unknown.

`Ast_FindMember()` then searches by name. `an_member` keeps the result for the code generator, and `an_type` takes the member's type so that `p->x + 1` checks as an `int` addition.

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

An lvalue is an expression that names an object rather than producing a value. A member access names one, since `p.x` designates storage inside `p` rather than a computed result.

`Sem_IsLvalue()` therefore gains `AST_NODE_KIND_MEMBER` as a third accepted kind. Assignment and the address-of operator both consult it, so `p.x = 1` and `&p.x` depend on the addition.

The test matters in both directions. Omitting the kind rejects correct code, which any test catches, while a test that is too permissive accepts `1 = p.x` and nothing catches that.

```c
// True if node names an object, so it can be assigned to or have its address taken.
int Sem_IsLvalue(const Ast_Node *node)
{
    return node->an_kind == AST_NODE_KIND_VAR || node->an_kind == AST_NODE_KIND_DEREF
        || node->an_kind == AST_NODE_KIND_MEMBER;
}
```

### Aggregate assignment

Assigning one whole structure to another is the only operation on a complete aggregate this stage supports. The copy belongs to the code generator, and this pass decides only whether it is allowed.

C defines no conversion between two structure types, so there is nothing to insert when they differ. Assignment between different aggregate types is therefore rejected rather than coerced.

The check guards on `Sem_IsAggregate()` so that scalars keep their existing conversions. It then compares `an_type` directly, which suffices because every reference to a tag resolves to one type object.

```c
/* Identity, not shape: two structs with the same members stay distinct. */
if (Sem_IsAggregate(node->an_lhs->an_type) && node->an_lhs->an_type != node->an_rhs->an_type) {
    Log_ShowErrorAt(node->an_line, "cannot assign a value of a different struct or union type");
}
```

### By-value arguments

Passing an aggregate by value requires the SysV ABI's argument classification algorithm, which this stage does not implement. Everything else about a call already works, which is what makes the gap dangerous.

The code generator will not refuse the case on its own. It leaves an aggregate as an address, so the callee reads its parameter from an address it was never given.

`Sem_CheckByValue()` therefore walks `an_args` at every call, testing each argument with `Sem_IsAggregate()`. The message names passing the address as the workaround, so the reader is not left guessing.

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

Returning an aggregate by value needs the same classification algorithm as passing one. There is only ever one value to examine, so the check sits in the `AST_NODE_KIND_RETURN` case rather than a function.

The generator would otherwise return the address of a local, which dies with its frame. That failure looks correct until the caller uses the value, and it reproduces inconsistently.

The `an_lhs` test comes first, because `return;` with no value is legal in a `void` function. Returning a pointer to an aggregate stays legal, since a pointer is not itself an aggregate.

```c
case AST_NODE_KIND_RETURN: {
    if (node->an_lhs && Sem_IsAggregate(node->an_lhs->an_type)) {
        Log_ShowErrorAt(node->an_line, "returning '%s' by value is not supported yet", Sem_TypeName(node->an_lhs->an_type));
    }
} break;
```

### Incomplete type dereference

The `AST_NODE_KIND_DEREF` case types a `*p` expression from `at_base`. Three things can make that invalid, and all three are tested before the type is assigned.

`Sem_IsPointer()` rejects an operand that is not a pointer, and the `AST_TYPE_KIND_VOID` test rejects a pointer to `void`, which has no size to load. Both predate this stage.

The `at_complete` test is the addition. A pointer to an incomplete type has no size and no member list, so `struct undefined *p; *p;` cannot be given a result type.

Catching it here is what makes the message useful. Without the check the complaint would come from the member lookup or the code generator, several steps from the missing definition.

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
