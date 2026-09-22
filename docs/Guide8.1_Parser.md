## Parser

A call site cannot tell what its callee returns, because the parser reduced a function's return type and then discarded it along with every prototype it read. The return type is now copied aside before a parameter overwrites it, and a prototype registers a function carrying a null body. The signature that keeps is what stage eight will check an indirect call against, so none of it is scaffolding.

### Extend: `decl_tail`

The return type sits in `Par_DeclType` when the parameter list opens, and the first parameter to reduce overwrites it. The mid-rule action copies it into `Par_CurRetType`, alongside the other state a definition accumulates before its body is read. Every per-definition value now lives in one group, so a later stage adding qualifiers or a calling convention extends the same place.

```c
decl_tail
    : LPAREN
        {
            Par_CurRetType    = Par_DeclType;  /* before a parameter overwrites it */

        }
      params RPAREN func_tail
    ;
```

### Add: `Par_MakeFunction()`

A definition and a prototype now both produce an `Ast_Func`, and writing that construction twice would let the two drift apart. `Par_MakeFunction()` builds it from the accumulated state and takes the body as an argument, asking the scope for locals only when a body exists. A later stage adding a field to `Ast_Func` therefore touches one place rather than two.

```c
/* Build the function the parser has just read a parameter list for. */
static Ast_Func *Par_MakeFunction(Ast_Node *body)
{
    Ast_Func *fn = calloc(1, sizeof(Ast_Func));

    fn->af_ret      = Par_CurRetType;
    fn->af_body     = body;
    fn->af_locals   = body ? Ast_CurrentLocals() : NULL;

    return fn;
}
```

### Modify: `func_tail`

The semicolon alternative closed the scope and returned, which discarded a prototype and left a forward call with nothing to consult. Both alternatives now register a function, and the semicolon passes a null body to mark the result as a declaration rather than a definition. The same mechanism carries a function defined in another translation unit, which is what makes separate compilation reachable.

```c
func_tail
    : compound_stmt
        { Par_AddFunction(Par_MakeFunction($1)); Ast_EndScope(); }
    | SEMI  /* a prototype: kept, so a call can find the return type */
        { Par_AddFunction(Par_MakeFunction(NULL)); Ast_EndScope(); }
    ;
```

### Modify: `Par_AddFunction()`

Keeping prototypes means one name can reach the program list twice, and appending both would leave a lookup finding the declaration. The function searches for the name before appending, and absorbs a later definition's body and locals into the entry it finds. A definition disagreeing with its declaration still goes unreported, which is a check that belongs here once types can be compared.

```c
/* Append a function to the program, or fill in one a prototype declared. */
static void Par_AddFunction(Ast_Func *fn)
{
    Ast_Func *seen = Par_FindFunction(fn->af_name);

    if (seen) {
        if (fn->af_body) {
            seen->af_body   = fn->af_body;
            seen->af_locals = fn->af_locals;
            seen->af_params = fn->af_params;
        }
        return;
    }

}
```
