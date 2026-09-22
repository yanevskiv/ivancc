## Ast

The tree recorded what every expression evaluates to but not what a function returns, because no function returned anything other than `int`. Two fields answer that, one holding the return type and one holding a frame slot for a structure a call hands back. No node kind joins them, since a call returning a structure is still a call and behaves like one everywhere else.

### Add: `Ast_Func.af_ret`

Nothing recorded what a function returned, so a call site could not decide whether its callee wants a hidden pointer. `af_ret` holds the type the parser captured, which the semantic pass reads to type a call and the code generator reads to place a frame. Recording a type rather than an ABI class keeps the parser free of the target, and stage eight reuses the field for a function pointer.

```c
struct Ast_Func {

    Ast_Type *af_ret;        // type the function returns
};
```

### Add: `Ast_Node.an_tmp`

A call returning an aggregate produces a value in registers, while every other aggregate expression produces an address. `an_tmp` is a frame slot on the call node, and the result is written there so that the call evaluates to an address like anything else. A slot per call rather than per function is what lets `sum(big(10))` work, since two aggregate results are live at the same moment there.

```c
struct Ast_Node {

    int          an_tmp;      // frame slot a CALL returning an aggregate lands in
};
```
