## Sem

The semantic pass can type scalar expressions, but aggregate member access and assignment need resolution and checks that the original tree cannot supply. It resolves members, recognizes aggregate lvalues, compares aggregate identities, and rejects by-value arguments and returns until ABI support exists. This stage keeps unsupported cases diagnostic, while the later ABI stage lifts the temporary by-value boundary.

### Extend: `case AST_NODE_KIND_MEMBER`

A member access arrives from the grammar with a name and an operand, and no idea which member that name refers to. The arm looks the name up in the operand's type and takes the member's type as its own. A complaint about an unknown member says nothing useful when the type was never defined in this translation unit.

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

### Extend: `Sem_IsLvalue()`

An lvalue is an expression that names an object, which `Sem_IsLvalue()` recognised for a variable and a dereference. The member node kind joins the two the function already accepts. `f().x = 1` is accepted for that reason, which C rejects and a later stage will have to as well.

```c
// True if node names an object, so it can be assigned to or have its address taken.
int Sem_IsLvalue(const Ast_Node *node)
{
    return node->an_kind == AST_NODE_KIND_VAR || node->an_kind == AST_NODE_KIND_DEREF
        || node->an_kind == AST_NODE_KIND_MEMBER;
}
```

### Extend: `case AST_NODE_KIND_ASSIGN`

Assignment converts its right operand to the type of its left, which every scalar pair reaches through a promotion. A guard on `Sem_IsAggregate()` compares the two types and rejects them when they differ. A pointer comparison suffices here since every reference to a tag resolves to one type object.

```c
case AST_NODE_KIND_ASSIGN: {

    /* Identity, not shape: two structs with the same members stay distinct. */
    if (Sem_IsAggregate(node->an_lhs->an_type) && node->an_lhs->an_type != node->an_rhs->an_type) {
        Log_ShowErrorAt(node->an_line, "cannot assign a value of a different struct or union type");
    }
    node->an_type = node->an_lhs->an_type;
} break;
```

### Add: `Sem_CheckByValue()`

Passing a structure by value needs the SysV argument classification algorithm, which decides what travels in a register. `Sem_CheckByValue()` walks a call's arguments and rejects any that is an aggregate. The message names passing the address as the workaround, so the restriction is discoverable rather than merely enforced.

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

### Extend: `case AST_NODE_KIND_RETURN`

Returning a structure by value needs the same classification algorithm that passing one does. The arm rejects a `return` whose expression is an aggregate, alongside the check on arguments. Returning a pointer to an aggregate stays legal too, because a pointer is not itself an aggregate.

```c
case AST_NODE_KIND_RETURN: {
    if (node->an_lhs && Sem_IsAggregate(node->an_lhs->an_type)) {
        Log_ShowErrorAt(node->an_line, "returning '%s' by value is not supported yet", Sem_TypeName(node->an_lhs->an_type));
    }
} break;
```

### Extend: `case AST_NODE_KIND_DEREF`

The dereference arm gives `*p` the type that `p` points at, which it reads from `at_base`. A test on `at_complete` joins the two checks the arm already performs. The check costs one flag read on an expression that is already being typed.

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
