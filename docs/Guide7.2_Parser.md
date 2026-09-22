## Parser

A declaration that names only a type, self-referential aggregate, or nested initializer needs grammar structure the original rules did not provide. The parser adds aggregate and enum specifiers, member access, and initializer productions while recording syntax for later passes rather than resolving types itself. This part therefore feeds the AST and semantic stages, which retain layout and name-resolution decisions.

### Extend: `external_decl`

A declaration ends at a declarator, which names the object it declares. The rule is factored so that the storage class and the type reduce on their own, leaving a tail that is either a semicolon or a declarator. A second rule for the bare case would share too long a prefix, forcing a choice the parser cannot yet make.

```c
external_decl
    : storage type_name
        { Par_DeclStorage = $1; Par_DeclType = $2; }
      external_tail
    ;

external_tail
    : SEMI                                /* `struct S { int x; };` ends here */
    | IDENT { Par_DeclName = $1; } decl_tail
    ;
```

### Add: `decl_body`

A block may contain a type-only declaration exactly as a translation unit may, since `struct S { int x; };` is ordinary C inside a function. `decl_body` expresses the bare case as an empty alternative of a rule of its own. A mid-rule action is itself a reduction, so bison would report a conflict rather than accepting the program.

```c
decl
    : storage type_name { Par_DeclType = $2; Par_DeclStorage = $1; } decl_body
    ;

decl_body
    : /* empty */          { $$ = NULL; } /* an alternative, not a second rule */
    | local_list           { $$ = $1; }
    ;
```

### Extend: `base`

`base` produces the primitive types, which every declaration reaches through it. Six alternatives join the rule, covering a tagged definition, an anonymous one, a bare tag reference and a typedef name. Binding the tag there rather than at the closing brace is what lets `struct node *next;` resolve inside `struct node`.

```c
base
    : INT                  { $$ = &Ast_TypeInt; }
    | CHAR                 { $$ = &Ast_TypeChar; }
    | VOID                 { $$ = &Ast_TypeVoid; }
    | struct_or_union tag_name LBRACE
        { $<type>$ = Par_BeginAggregate($1, $2, @2); }   /* bind the tag first */
      members RBRACE
        { Ast_LayoutAggregate($<type>4, $5, @1); $$ = $<type>4; }
    | struct_or_union LBRACE
        { $<type>$ = Par_BeginAggregate($1, NULL, @1); } /* anonymous: no tag */
      members RBRACE
        { Ast_LayoutAggregate($<type>3, $4, @1); $$ = $<type>3; }
    | struct_or_union tag_name
        { $$ = Par_ReferenceAggregate($1, $2, @2); }     /* a bare reference */
    | TYPEDEF_NAME         { $$ = Ast_FindTypedef($1); }
    ;

struct_or_union
    : STRUCT               { $$ = AST_TYPE_KIND_STRUCT; }
    | UNION                { $$ = AST_TYPE_KIND_UNION; }
    ;
```

### Add: `Par_BeginAggregate()`

An aggregate definition creates a type and binds its tag, which the member list then fills in. `Par_BeginAggregate()` tells the three apart and reuses, shadows or rejects accordingly. The kind test catches `union S` after `struct S`, which would otherwise share a tag and take whichever layout came first.

```c
static Ast_Type *Par_BeginAggregate(Ast_TypeKind kind, const char *tag, int line)
{
    Ast_Type *type = tag ? Ast_FindTagHere(tag) : NULL; /* innermost scope only */

    if (type && type->at_complete) {
        Log_ShowErrorAt(line, "redefinition of '%s'", tag);
    }
    if (type && type->at_kind != kind) {
        Log_ShowErrorAt(line, "'%s' was declared with a different aggregate keyword", tag);
    }
    if (! type) {
        type = Ast_NewAggregate(kind, tag);
        if (tag) {
            Ast_DeclareTag(tag, type);
        }
    }
    return type;
}
```

### Add: `Par_ReferenceAggregate()`

A bare tag reference names an aggregate without defining it, as `struct node` does in `struct node *p;`. `Par_ReferenceAggregate()` creates an incomplete type for an unknown tag and binds it immediately. A pointer to an incomplete type has a known size, which is what lets the declaration be laid out anyway.

```c
static Ast_Type *Par_ReferenceAggregate(Ast_TypeKind kind, const char *tag, int line)
{
    Ast_Type *type = Ast_FindTag(tag);    /* every enclosing scope this time */

    if (type && type->at_kind != kind) {
        Log_ShowErrorAt(line, "'%s' was declared with a different aggregate keyword", tag);
    }
    if (! type) {
        type = Ast_NewAggregate(kind, tag); /* incomplete is enough to point at */
        Ast_DeclareTag(tag, type);
    }
    return type;
}
```

### Add: `tag_name`

Tags and ordinary identifiers occupy separate namespaces in C, so a tag may reuse a name a typedef has bound. `tag_name` accepts either token and yields the same string. Leaving the alternative out produces a syntax error pointing at a declaration that is correct C.

```c
tag_name
    : IDENT                { $$ = $1; }
    | TYPEDEF_NAME         { $$ = $1; } /* `typedef struct node node;` */
    ;
```

### Add: `members`

A member declaration is a type specifier followed by declarators, and an aggregate is a list of them. `members` accumulates the declarations and `member_declarator` collects a name and its dimensions without building anything. Empty brackets are a separate alternative, since a flexible array member is not a dimension list of length zero.

```c
members
    : /* empty */          { $$ = NULL; }
    | members member_decl  { $$ = Par_AppendMembers($1, $2); }
    ;

member_decl
    : type_name member_declarators SEMI  { $$ = Par_MakeMembers($1, $2); }
    ;

member_declarator
    : IDENT array_dims
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_NOP, @1);
          n->an_memname = $1; n->an_lhs = $2; $$ = n; }
    | IDENT LSQUARE RSQUARE                     /* `char data[]` */
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_NOP, @1);
          n->an_memname = $1; n->an_val = 1; $$ = n; }
    ;
```

### Add: `Par_MakeMembers()`

The collected declarators have to become members, each applying its own dimensions to the shared type. `Par_MakeMembers()` walks the declarators and builds one member from each through `Par_ArrayType()`. `y` would come out as a `struct B`, and the size of `struct A` would be wrong with no diagnostic anywhere.

```c
static Ast_Member *Par_MakeMembers(Ast_Type *type, Ast_Node *decls)
{
    Ast_Member head = {0};
    Ast_Member *tail = &head;

    for (Ast_Node *decl = decls; decl; decl = decl->an_next) {
        /* Each declarator applies its own dimensions to the shared type. */
        tail->am_next = Ast_NewMember(decl->an_memname, Par_ArrayType(type, decl->an_lhs), decl->an_line);
        tail = tail->am_next;
        if (decl->an_val) {
            tail->am_type = Ast_NewArray(type, 0);      /* length zero */
            tail->am_flexible = 1;
        }
    }
    return head.am_next;
}
```

### Add: `Par_EnumValue`

An enum specifier declares a set of named integer constants and yields a type. `Par_EnumValue` is that counter, reset by a mid-rule action at the opening brace. The type the specifier yields is `int`, since C permits any compatible integer type and nothing downstream then has to know enums exist.

```c
base

    | ENUM tag_name LBRACE { Par_EnumValue = 0; } enumerators RBRACE
        { Ast_DeclareTag($2, &Ast_TypeInt); $$ = &Ast_TypeInt; }
    | ENUM LBRACE { Par_EnumValue = 0; } enumerators RBRACE
        { $$ = &Ast_TypeInt; }
    | ENUM tag_name        { $$ = &Ast_TypeInt; }
    ;
```

### Add: `Par_AddEnumConst()`

Each enumerator is declared at the counter's current value, after which the counter steps. `Par_AddEnumConst()` declares the name and steps `Par_EnumValue` in one expression. The test admits `= 10` and rejects `= 1 + 1`, naming the enumerator so the restriction is discoverable rather than merely enforced.

```c
static void Par_AddEnumConst(const char *name, Ast_Node *value, int line)
{
    if (value) {
        /* No constant folder yet: the parser must already hold a number. */
        if (value->an_kind != AST_NODE_KIND_NUM) {
            Log_ShowErrorAt(line, "enumerator '%s' is not a constant", name);
        }
        Par_EnumValue = value->an_val;
    }
    Ast_DeclareEnumConst(name, Par_EnumValue++);
}
```

### Extend: `primary`

`primary` reads an identifier and normally looks it up as a variable, but enumeration constants are values with no storage to load. The rule consults `Ast_FindEnumConst()` before `Ast_FindVar()` and replaces a match with a number, so constants work in expressions and case labels. The ordinary variable path remains second, preserving existing identifiers while the semantic stage receives literal-like enum nodes.

```c
primary
    : IDENT
        { long val;
          /* Enum constants first: they are numbers, not objects. */
          if (Ast_FindEnumConst($1, &val)) { $$ = Ast_NewNum(val, @1); }
          else {
              Ast_Var *v = Ast_FindVar($1);
              if (! v) Log_ShowErrorAt(@1, "use of undeclared identifier '%s'", $1);
              $$ = Ast_NewVarNode(v, @1);
          } }
    ;
```

### Add: `array_len`

An array bound is read by a rule that accepts a numeric literal, which is what every earlier stage needed. `array_len` accepts a literal or an identifier and folds the identifier through `Ast_FindEnumConst()`. The value is produced as a number rather than a node, since a dimension is not an expression and the rules above expect a `long`.

```c
array_len
    : NUM                  { $$ = $1; }
    | IDENT                                     /* `int table[GONE];` */
        { long val;
          if (! Ast_FindEnumConst($1, &val)) {
              Log_ShowErrorAt(@1, "'%s' is not a constant", $1);
          }
          $$ = val; }
    ;
```

### Extend: `storage`

A typedef declaration has the same shape as an object declaration, differing by one keyword. `storage` gains a `TYPEDEF` alternative beside `STATIC` and `EXTERN`. The empty alternative stays first, so a declaration with no storage class still reduces without a conflict.

```c
storage
    : /* empty */          { $$ = AST_STORAGE_NONE; }
    | STATIC               { $$ = AST_STORAGE_STATIC; }
    | EXTERN               { $$ = AST_STORAGE_EXTERN; }
    | TYPEDEF              { $$ = AST_STORAGE_TYPEDEF; }
    ;
```

### Extend: `Par_AddGlobal()`

`Par_AddGlobal()` turns a declarator into a declaration, which until now always reserved storage. The function branches on the storage class and binds through `Ast_DeclareTypedef()` under `TYPEDEF`. Registration happens per declarator rather than at the semicolon, which is what makes `typedef int Foo, Bar;` work with the lexer.

```c
static void Par_AddGlobal(const char *name, Ast_Node *dims, Ast_Node *init, int line)
{
    if (Par_DeclStorage == AST_STORAGE_TYPEDEF) {
        /* The array dimensions go into the bound type, not into an object. */
        Ast_DeclareTypedef(name, Par_ArrayType(Par_DeclType, dims));
        return;
    }

}
```

### Extend: `postfix`

`postfix` holds the operators that bind more tightly than any prefix one, such as a call and a subscript. Two alternatives join the rule, and the `->` one wraps its operand in a dereference. The semantic pass, the lvalue test and the code generator would each need a second case otherwise, doing almost the same thing.

```c
postfix

    | postfix DOT IDENT    { $$ = Ast_NewMemberNode($1, $3, @2); }
    | postfix ARROW IDENT  /* built as `(*a).b` */
        { $$ = Ast_NewMemberNode(Ast_NewUnary(AST_NODE_KIND_DEREF, $1, @2), $3, @2); }
    ;
```

### Add: `Ast_NewMemberNode()`

A member access reduces with a name and an operand, and the member that name refers to has to be found. `Ast_NewMemberNode()` stores the name and leaves the member pointer null. Without the copy the node would name whatever token happened to follow, and the resulting diagnostics would point anywhere but the fault.

```c
// Build a member access, whose member the Sem_ pass resolves once it has a type.
Ast_Node *Ast_NewMemberNode(Ast_Node *lhs, const char *name, int line)
{
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_MEMBER, lhs, line);
    node->an_memname = strdup(name);    /* resolved later, in Sem_ */
    return node;
}
```

### Add: `initializer`

An initializer for an aggregate can nest, omit braces and aim an item at a subobject, and all three can appear in one. `initializer` records what was written and interprets none of it, building a node for a braced list and a list of designators per item. `an_cond` is borrowed to carry the designators, since an initializer item has no condition of its own to keep there.

```c
initializer
    : expr                       { $$ = $1; }
    | LBRACE init_list RBRACE    /* a list is its own node, not a scalar */
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_INITLIST, @1); n->an_body = $2; $$ = n; }
    ;

init_item
    : initializer                { $$ = Ast_NewUnary(AST_NODE_KIND_INIT, $1, @1); }
    | designators ASSIGN initializer
        { Ast_Node *n = Ast_NewUnary(AST_NODE_KIND_INIT, $3, @1); n->an_cond = $1; $$ = n; }
    ;

designator
    : LSQUARE array_len RSQUARE  /* `[4] = ...` */
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_DESIGNATOR, @1); n->an_val = $2; $$ = n; }
    | DOT IDENT                  /* `.y = ...` */
        { Ast_Node *n = Ast_NewNode(AST_NODE_KIND_DESIGNATOR, @1); n->an_memname = $2; $$ = n; }
    ;
```

### Add: `Par_FlattenList()`

Element indices cannot describe an initializer where a designator names a member of a member. `Par_FlattenList()` walks the recorded initializer against the type and produces one entry per scalar. A union clears its cursor rather than stepping it, so a second item is reported instead of overwriting the first.

```c
static void Par_FlattenList(Ast_Type *type, int base, Ast_Node **item, Ast_Node **tail, int braced, int line)
{
    int index = 0;
    Ast_Member *member = type->at_members;

    while (*item) {
        if ((*item)->an_cond) {
            if (! braced) {
                return;                       /* elided brace: hand the item back */
            }
            int off = base;
            Ast_Node *desig = (*item)->an_cond;
            Ast_Type *slot = type;

            /* Move the cursor, then step type and offset once per designator. */
            Par_Designate(type, desig, &index, &member, line);
            Par_Step(&slot, &off, desig, index, member);
            for (Ast_Node *next = desig->an_next; next; next = next->an_next) {
                int at = 0;
                Ast_Member *inner = NULL;
                Par_Designate(slot, next, &at, &inner, line);
                Par_Step(&slot, &off, next, at, inner);
            }

            (*item)->an_cond = NULL;
            Par_Flatten(slot, off, (*item)->an_lhs, tail, line);
            *item = (*item)->an_next;
            if (desig->an_memname) {
                member = type->at_kind == AST_TYPE_KIND_UNION ? NULL : member->am_next;
            } else {
                index++;
            }
            continue;
        }

        if (type->at_kind == AST_TYPE_KIND_ARRAY) {
            if (index >= type->at_len) {
                if (! braced) {
                    return;
                }
                Log_ShowErrorAt(line, "too many initializers for an array of %d", type->at_len);
            }
            Par_FlattenSlot(type->at_base, base + index * type->at_base->at_size, item, tail, line);
            index++;
            continue;
        }

        if (! member) {
            if (! braced) {
                return;
            }
            Log_ShowErrorAt(line, "too many initializers for '%s'", Sem_TypeName(type));
        }
        Par_FlattenSlot(member->am_type, base + member->am_offset, item, tail, line);
        member = type->at_kind == AST_TYPE_KIND_UNION ? NULL : member->am_next;
    }
}
```

### Add: `Par_FlattenSlot()`

C allows the inner braces of a nested initializer to be left out, so `int e[2][2] = {1, 2, 3, 4};` means `{{1, 2}, {3, 4}}`. `Par_FlattenSlot()` decides from one flag threaded through the walk, recursing into an unbraced slot with the same cursor. That flag also decides what an overrun means, since a braced list receiving too many items is an error and an unbraced one has simply finished.

```c
static void Par_FlattenSlot(Ast_Type *type, int base, Ast_Node **item, Ast_Node **tail, int line)
{
    Ast_Node *value = (*item)->an_lhs;

    if (value->an_kind == AST_NODE_KIND_INITLIST) {
        Par_Flatten(type, base, value, tail, line);       /* the source braced it */
        *item = (*item)->an_next;
        return;
    }
    if (type->at_kind == AST_TYPE_KIND_ARRAY || Sem_IsAggregate(type)) {
        Par_FlattenList(type, base, item, tail, 0, line); /* elided: share the cursor */
        return;
    }
    (*tail)->an_next = Par_InitAt(base, type, value, line);
    *tail = (*tail)->an_next;
    *item = (*item)->an_next;
}
```

### Add: `Par_Step()`

A designator chain such as `.b.y` names a subobject several levels down. `Par_Step()` adds one designator's offset and narrows the type, leaving both ready for the next. Splitting the two is what keeps finding a target separate from computing where it sits.

```c
static void Par_Step(Ast_Type **type, int *off, Ast_Node *desig, int index, Ast_Member *member)
{
    if (desig->an_memname) {
        *off += member->am_offset;              /* `.b` */
        *type = member->am_type;
        return;
    }
    *off += index * (*type)->at_base->at_size;  /* `[i]` */
    *type = (*type)->at_base;
}
```

### Add: `Par_InitStore()`

A flattened entry has to become a write, which a global performs into an image at compile time. `Par_InitStore()` builds one store statement per entry, addressed through a cast to `char *`. The second cast restores the slot's own type, so the entry rather than the object decides the width.

```c
static Ast_Node *Par_InitStore(Ast_Var *var, int off, Ast_Type *type, Ast_Node *value, int line)
{
    /* `(char *) &var` so that `+ off` counts bytes, not elements. */
    Ast_Node *addr = Ast_NewUnary(AST_NODE_KIND_CAST, Ast_NewUnary(AST_NODE_KIND_ADDR, Ast_NewVarNode(var, line), line), line);
    addr->an_type = Ast_NewPointer(&Ast_TypeChar);

    Ast_Node *at = Ast_NewUnary(AST_NODE_KIND_CAST, Ast_NewBinary(AST_NODE_KIND_ADD, addr, Ast_NewNum(off, line), line), line);
    at->an_type = Ast_NewPointer(type);         /* back to the slot's own type */

    Ast_Node *slot = Ast_NewUnary(AST_NODE_KIND_DEREF, at, line);
    return Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, Ast_NewBinary(AST_NODE_KIND_ASSIGN, slot, value, line), line);
}
```

### Add: `Par_InitLocal()`

C says whatever an initializer omits is zero, and for an aggregate the omissions can be anywhere inside it. `Par_InitLocal()` emits one statement clearing the whole object, then chains the stores onto it. The function returns the zero statement because that node heads the list the caller inserts.

```c
static Ast_Node *Par_InitLocal(Ast_Var *var, Ast_Node *init, int line)
{
    if (init->an_kind != AST_NODE_KIND_INITLIST && var->av_type->at_kind != AST_TYPE_KIND_ARRAY) {
        Ast_Node *assign = Ast_NewBinary(AST_NODE_KIND_ASSIGN, Ast_NewVarNode(var, line), init, line);
        return Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, assign, line);
    }

    Ast_Node *zero = Ast_NewUnary(AST_NODE_KIND_ZERO, Ast_NewVarNode(var, line), line);
    zero->an_val = var->av_type->at_size;       /* clear the gaps first */

    Ast_Node *tail = zero;
    for (Ast_Node *item = Par_FlattenInit(var->av_type, init, line); item; item = item->an_next) {
        tail->an_next = Par_InitStore(var, (int) item->an_val, item->an_type, item->an_lhs, line);
        tail = tail->an_next;
    }
    return zero;
}
```
