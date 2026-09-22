## Sem

The pass typed every call as `int` and rejected every aggregate argument, which is exactly the pair this test has to undo. The arm that types a call now reads the callee's return type, and the two by-value rejections are deleted along with the function that held one. Checking a definition against its declaration is what belongs here next, once types can be compared.

### Modify: `case AST_NODE_KIND_CALL`

The arm assigned `&Ast_TypeInt` unconditionally, so `struct Point s = add(p, q);` failed the identity test that aggregate assignment performs. It now looks the callee up and takes `af_ret` from it, falling back to `int` for a name this translation unit never mentions. Narrowing that fallback to a diagnostic is what stage eight needs, once every call has a prototype to check against.

```c
case AST_NODE_KIND_CALL: {
    Ast_Func *func = Sem_FindFunc(node->an_funcname);
    Sem_CheckCall(node);
    node->an_type = func && func->af_ret ? func->af_ret : &Ast_TypeInt;
} break;
```

### Extend: `Sem_Analyze()`

A prototype now sits in the program list with a null body, which the walk over every function would dereference. A guard at the top of the loop skips any function without one, before `Sem_CurFunc` is set and before anything reads it. The code generator's loop skips a prototype the same way, and a check of a definition against its declaration would go here.

```c
// Annotate every node with its type and reject what the grammar cannot.
void Sem_Analyze(Ast_Func *prog)
{
    Sem_Prog = prog;

    for (Ast_Func *func = prog; func; func = func->af_next) {
        if (! func->af_body) {
            continue;  // a prototype declares a signature and nothing to walk
        }

    }
}
```

### Delete: `case AST_NODE_KIND_RETURN`

The arm rejected a `return` carrying an aggregate, and `Sem_CheckByValue()` rejected an aggregate argument, both naming the classification algorithm as what was missing. Both checks are deleted, and the label joins the statement kinds this pass only walks so that a `return` still has its expression typed. Checking that expression against `af_ret` is what belongs here instead, once conversions exist to describe what may be returned.

```c
case AST_NODE_KIND_RETURN:

case AST_NODE_KIND_FOR: {
    // nothing to check
} break;
```
