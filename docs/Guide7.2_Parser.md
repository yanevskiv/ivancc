## Parser

Parser work divides into four largely independent pieces. They are declarations that name only a type, the aggregate and enum specifiers themselves, member access, and a rebuilt initializer syntax. The first is a precondition for the second, and the initializer work is by far the largest of the four.

### Type-only declarations

A type-only declaration names a type and declares no object. `struct S { int x; };` is one such declaration, and `enum Color { RED };` is another, yet neither has a declarator anywhere in it.

A rule written as `storage type_name IDENT ...` cannot express either of them, since it requires a declarator before the semicolon. A second rule for the bare case would share a long prefix, forcing the choice too early.

`external_decl` therefore reduces the storage class and the type specifier alone. Its mid-rule action stores them in `Par_DeclStorage` and `Par_DeclType`, because the declarator actions run after those stack positions are gone.

`external_tail` takes the choice one token later. Its `SEMI` alternative ends the declaration with no declarator, while its `IDENT` alternative records the name and continues into `decl_tail` for pointers, arrays, and initializers.

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

### Local declarations

A block may contain a type-only declaration exactly as a translation unit may. `struct S { int x; };` inside a function body is ordinary C, so the local rules need the same factoring.

`decl` mirrors `external_decl`, filling the same two globals from a mid-rule action. The two rules stay separate because a local declaration produces a statement list for its block while a file-scope one produces nothing.

The bare case cannot be a second rule for `decl`. A mid-rule action is itself a reduction, so bison must choose between two rules sharing a prefix before the distinguishing token has been read.

`decl_body` expresses the bare case as an empty alternative instead. The mid-rule action then belongs to one rule only, so the parser reduces it unconditionally and decides afterwards what follows.

```c
decl
    : storage type_name { Par_DeclType = $2; Par_DeclStorage = $1; } decl_body
    ;

decl_body
    : /* empty */          { $$ = NULL; } /* an alternative, not a second rule */
    | local_list           { $$ = $1; }
    ;
```

### Struct and union specifiers

A struct or union specifier is whatever names an aggregate type in a declaration. Three forms exist: a definition with a tag, an anonymous definition, and a bare reference, and all three belong in `base` beside `int` and `char`.

A structure may contain a pointer to its own type, as `struct node { struct node *next; };` does. The member list therefore refers to a type that the specifier producing it has not finished building.

The two defining alternatives split the work across two actions. A mid-rule action calls `Par_BeginAggregate()` at the opening brace, so the tag is bound while the member list is still being read.

`Ast_LayoutAggregate()` runs in the closing action, taking the type back out of `$<type>`. A single action after `RBRACE` could not work, because the type would not exist while the members were read.

The third alternative calls `Par_ReferenceAggregate()` and names no member list, covering `struct node *p;` and a forward declaration alike. The `TYPEDEF_NAME` alternative beside it resolves an already bound name.

`struct_or_union` collapses two keywords into one value. Every alternative above takes that kind as a parameter, so none of the three forms has to be written out twice.

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

### Aggregate definitions

An aggregate definition is a specifier carrying a member list, such as `struct Point { int x; int y; };`. Reading one creates a type, binds its tag, and lays out its members, of which `Par_BeginAggregate()` does the first two.

The tag may already be bound when the definition is read. Three things can have bound it: a forward declaration, a definition in an enclosing scope, or an earlier definition in this same scope.

`Ast_FindTagHere()` separates the second case from the other two. It searches the innermost scope alone, so a definition inside a block shadows an outer one rather than completing it.

The `at_complete` flag separates the first case from the third. An incomplete type came from a forward declaration and is reused, while a complete one came from an earlier definition and is a redefinition.

The kind test guards against `union Point` following `struct Point`. Tags are stored without regard to the keyword, so such a pair would otherwise share one tag and one arbitrary layout.

`Ast_NewAggregate()` creates a type when none can be reused, and `Ast_DeclareTag()` binds it. An anonymous definition skips the binding, since a null `tag` leaves nothing for a later reference to name.

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

### Tag references

A tag reference is a specifier that names an aggregate without defining it. `struct node` in `struct node *p;` is one example, and `Par_ReferenceAggregate()` turns it into a type without reading any member list.

The tag may never have been declared anywhere. A program may write `struct list *head;` before `struct list` exists, and rejecting that would break forward declarations and self-referential structures alike.

An unknown tag therefore gets an incomplete type from `Ast_NewAggregate()`, bound immediately so a second reference finds the same one. A pointer to an incomplete type has a known size, so the declaration still lays out.

`Ast_FindTag()` searches every enclosing scope rather than the innermost alone, since a file-scope tag must stay visible inside a function. That is the one difference from the lookup a definition performs.

The kind test repeats the one in `Par_BeginAggregate()`, because a reference can disagree with a definition. `union node *p;` after `struct node { ... };` is caught here rather than at the member access that follows.

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

### Tags as typedef names

Tags and ordinary identifiers occupy separate namespaces in C, so a tag may reuse a name a typedef has bound. `typedef struct node node;` does exactly that, and the idiom is common.

The lexer's typedef feedback does not respect that separation. Once `node` is bound, every later occurrence arrives as `TYPEDEF_NAME`, which a tag position expecting `IDENT` will not accept.

`tag_name` therefore accepts either token and yields the same string. The actions above it never learn which one arrived, and `struct node` keeps working because of this one rule.

```c
tag_name
    : IDENT                { $$ = $1; }
    | TYPEDEF_NAME         { $$ = $1; } /* `typedef struct node node;` */
    ;
```

### Member declarations

A member declaration is one line of an aggregate's member list: a type specifier, one or more declarators, and a semicolon. `int x, *p, a[4];` declares three members of three different types from one specifier.

Those declarators share one specifier and produce different types, so a declarator cannot build its type as it reduces. Each one's name and dimensions are collected instead, and the members built afterwards.

`members` accumulates the declarations of one aggregate. Its empty alternative lets the grammar reach `Ast_LayoutAggregate()` with a null list, and `Par_AppendMembers()` appends rather than prepends to preserve declaration order.

`member_decl` is one declaration and builds nothing itself. Both parts go to `Par_MakeMembers()`, which is where the shared type is distributed across the declarators that need it.

`member_declarator` collects a name and its dimensions without building a type. Its first alternative carries `array_dims` through `an_lhs`, and its second matches empty brackets and records that in `an_val`.

Both alternatives build an `AST_NODE_KIND_NOP` node as a carrier for a name and a dimension list. Reusing an existing kind avoids adding one that two rules produce and `Par_MakeMembers()` immediately discards.

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

### Member construction

`Par_MakeMembers()` turns the collected declarators into a member list. It runs once per member declaration, with the shared type passed in as an argument rather than read from a global.

`Par_ArrayType()` applies each declarator's dimensions to that shared type. It returns a new type rather than modifying the old one, so the next declarator still sees the shared type unchanged.

The `an_val` test picks out a flexible array member, which was given no dimensions at all. Its type is replaced with `Ast_NewArray(type, 0)`, and `am_flexible` is set for the layout pass to act on.

The `head` and `tail` pair builds the list in order without a special case for the first member. `head` is a dummy whose `am_next` becomes the result, which is why the function returns `head.am_next`.

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

### Nested member lists

A member declaration may itself define an aggregate, since a structure definition is a type specifier. One member list can therefore open while another is still being read, as the example below does.

`struct B` is defined inside `struct A`'s member list, so the inner `member_decl` reduces while the outer `members` is still accumulating. `y` follows that definition and must still come out as an `int`.

A single "type of the declaration being parsed" global would be overwritten as `struct B` reduced. `y` would become a `struct B`, and the size of `struct A` would be wrong with no diagnostic.

Passing the type to `Par_MakeMembers()` as an argument avoids that entirely. Each member declaration carries its own type down the parser stack, where the inner one cannot reach it.

```c
struct A {
    struct B { int x; } b; /* opens and closes a second member list */
    int y;                 /* must still be int, not struct B */
};
```

### Enum specifiers

An enum specifier declares a set of named integer constants and also yields a type. `enum Color { RED, GREEN, BLUE };` declares three constants counting from zero, and allows `enum Color c;` afterwards.

That type needs no kind of its own. C lets the implementation choose any compatible integer type, and `int` is the simplest honest choice, so nothing downstream learns that enums exist.

The tagged alternative calls `Ast_DeclareTag()` with `&Ast_TypeInt`, so a later `enum Color c;` resolves through the ordinary tag lookup. The tag exists purely so the name can be written again.

The mid-rule action resetting `Par_EnumValue` runs at the opening brace, before `enumerators` is parsed. Resetting after the list instead would work for one enum and fail for the second.

The third alternative yields `&Ast_TypeInt` without consulting the tag at all. A reference to an undeclared enum tag is therefore accepted as `int`, which is a deliberate simplification here.

```c
base
    : /* ... INT, CHAR, VOID and the aggregate specifiers ... */
    | ENUM tag_name LBRACE { Par_EnumValue = 0; } enumerators RBRACE
        { Ast_DeclareTag($2, &Ast_TypeInt); $$ = &Ast_TypeInt; }
    | ENUM LBRACE { Par_EnumValue = 0; } enumerators RBRACE
        { $$ = &Ast_TypeInt; }
    | ENUM tag_name        { $$ = &Ast_TypeInt; }
    ;
```

### Enumerators

An enumerator is one name within an enum specifier's braces, declared at the current value of a running counter. `Par_AddEnumConst()` performs that declaration and then steps the counter by one.

An explicit value such as `FAILED = 20` becomes the new running value rather than applying to one name alone. An implementation treating it as a one-off would give the following `GONE` the value 2.

An explicit value must be a constant expression, and this stage has no folder. The `AST_NODE_KIND_NUM` test therefore admits `= 10` and rejects `= 1 + 1`, naming the enumerator so the restriction is discoverable.

`Ast_DeclareEnumConst()` receives `Par_EnumValue`, and the post-increment steps it in the same expression. The constant lands in the scope rather than in any type, which is where the expression rules look for it.

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

### Constant folding in expressions

An enumeration constant is a value rather than an object. `RED` stands for the number 0, and no storage was ever allocated for it, so there is no address to load from.

The grammar reads `RED` through the same rule that reads a variable. Left alone, that rule calls `Ast_FindVar()` and reports an undeclared identifier for a correctly declared constant.

`Ast_FindEnumConst()` is therefore consulted first, before `Ast_FindVar()` runs at all. A name it recognises becomes `Ast_NewNum()`, which is indistinguishable from a literal for everything downstream.

The `else` branch is the ordinary variable path, unchanged except that it runs second. A name that is neither still reports as undeclared, at the cost of one failed search.

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

### Constant folding in array lengths

An array bound is the other place an enumeration constant is commonly used. `int table[GONE];` sizes an array from an enumerator, which is most of the reason to declare the constant at all.

A dimension rule that accepts only `NUM` rejects that declaration. The fold in the expression rules does not help, because a dimension is read by a rule that never reaches `primary`.

`array_len` therefore accepts both forms in a rule of its own. `NUM` stays as the first alternative, so `int a[4];` reduces exactly as it did and nothing that already worked changes shape.

The `IDENT` alternative folds through `Ast_FindEnumConst()` and rejects anything else. The value is produced as a number, so the rules above `array_len` continue to receive a `long`.

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

### typedef as a storage class

A typedef declaration has the same shape as an object declaration. `typedef char *String;` and `char *s;` differ by one keyword, and only what the declaration means at the end differs.

`storage` therefore gains a `TYPEDEF` alternative beside `STATIC` and `EXTERN`. The declarator rules are reached without any change of their own, and the decision moves into the action that already branches on storage.

Pointers and arrays consequently come along for free, since the declarator already handles both for objects. `typedef int Row[4];` reduces through the same path as `int row[4];`.

The empty alternative stays where it is, so a declaration with no storage class still reduces. Adding `TYPEDEF` introduces no conflict, because the alternatives are distinguished by their first token.

```c
storage
    : /* empty */          { $$ = AST_STORAGE_NONE; }
    | STATIC               { $$ = AST_STORAGE_STATIC; }
    | EXTERN               { $$ = AST_STORAGE_EXTERN; }
    | TYPEDEF              { $$ = AST_STORAGE_TYPEDEF; }
    ;
```

### Typedef name binding

`Par_AddGlobal()` is where a declarator's name becomes a declaration. It runs once per declarator, and what it declares depends on the storage class that `storage` recorded several tokens earlier.

Under `AST_STORAGE_TYPEDEF` it binds the name through `Ast_DeclareTypedef()` and returns. No object is created and no storage reserved, and the early return leaves the rest of the function untouched.

`Par_ArrayType()` runs before that binding, so everything the declarator built goes into the bound type. Dropping it would bind `Row` in `typedef int Row[4];` to `int` and lose the dimensions silently.

Registration happens here, during each declarator's reduction rather than at the end of the declaration. `typedef int Foo, Bar;` needs `Foo` registered before the lexer reads `Bar`.

The rule that declares a local needs the same branch, since a typedef may appear inside a function body. `Par_DeclareLocal()` tests the same storage class and calls the same binding function.

```c
static void Par_AddGlobal(const char *name, Ast_Node *dims, Ast_Node *init, int line)
{
    if (Par_DeclStorage == AST_STORAGE_TYPEDEF) {
        /* The array dimensions go into the bound type, not into an object. */
        Ast_DeclareTypedef(name, Par_ArrayType(Par_DeclType, dims));
        return;
    }
    /* ... the ordinary object declaration, unchanged ... */
}
```

### Member access operators

`.` and `->` are the two ways of reaching a member. `p.x` reads one out of an object, while `p->x` reads one through a pointer, and C defines the second form in terms of the first.

Both belong at the postfix level. They bind more tightly than any prefix operator, so `&p.x` takes the address of the member, and they chain left to right, so `a.b.c` parses as `(a.b).c`.

The `DOT` alternative passes its left operand straight to `Ast_NewMemberNode()`. Left recursion on `postfix` produces the chaining, since the result of one access is itself a `postfix` for the next.

The `ARROW` alternative wraps the operand in `AST_NODE_KIND_DEREF` first, building `(*a).b` directly. The semantic pass, the lvalue test, and the code generator then handle one node kind instead of two.

```c
postfix
    : /* ... calls, subscripts, postfix ++ and -- ... */
    | postfix DOT IDENT    { $$ = Ast_NewMemberNode($1, $3, @2); }
    | postfix ARROW IDENT  /* built as `(*a).b` */
        { $$ = Ast_NewMemberNode(Ast_NewUnary(AST_NODE_KIND_DEREF, $1, @2), $3, @2); }
    ;
```

### Deferred member resolution

`Ast_NewMemberNode()` builds the node a member access reduces to, from the left operand and the name after the operator. What it does not do is find the member itself.

Finding the member requires the type of the left operand, which is not known while the expression reduces. `p->x` gives the parser an expression and a name with nothing to connect them.

`Ast_NewUnary()` therefore builds the node with the member pointer left null. Nothing between the parser and the semantic pass reads that pointer, so the window during which it is null is harmless.

`strdup()` copies the name into `an_memname` rather than keeping the lexer's pointer. The lexer reuses its buffer immediately, so without the copy the node would name whatever token followed.

```c
// Build a member access, whose member the Sem_ pass resolves once it has a type.
Ast_Node *Ast_NewMemberNode(Ast_Node *lhs, const char *name, int line)
{
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_MEMBER, lhs, line);
    node->an_memname = strdup(name);    /* resolved later, in Sem_ */
    return node;
}
```

### Initializer grammar

An initializer for an aggregate can take several forms at once. It can nest as `{{1, 2}, {3, 4}}`, omit braces as `{1, 2, 3, 4}`, and aim an item at a subobject as `{.b.y = 9}`.

Element indices cannot express all three together. A designator can name a member of a member, and an elided brace can cross a nesting level, so the index that should advance is not always the one in hand.

Flattening everything to byte offsets removes the difficulty. Every item becomes an offset, a type, and a value, so a global and a local share one computation.

The grammar records what was written and interprets none of it. Resolution needs the type being initialized, which is not available where these rules reduce, so every decision is left to the walk.

`initializer` is either an expression or a braced list, and the braced case builds an `AST_NODE_KIND_INITLIST` node. That node is what later distinguishes a nested list from a scalar in the same position.

`init_item` wraps each item in `AST_NODE_KIND_INIT` and hangs any designators on `an_cond`. The field is borrowed rather than added, since an initializer item has no condition of its own.

`designator` records `[4]` in `an_val` and `.y` in `an_memname` and nothing more. Resolving either would need the type of the enclosing object, which the grammar does not have here.

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

### Initializer flattening

`Par_FlattenList()` walks the recorded initializer against the type being initialized. It produces one entry per scalar, carrying a byte offset, the slot's type, and the value that belongs there.

The walk carries a cursor over the type, with `index` for an array and `member` for a structure. Exactly one of the two is meaningful at a time, decided by the type's kind.

The `an_cond` branch resolves a designator chain before placing anything. `Par_Designate()` moves the cursor and `Par_Step()` accumulates the offset, looping over `an_next` for each further designator.

The array branch places an item at `index * at_size` and steps the index. An index past `at_len` is an error in a braced list and a signal to return in an elided one.

The member branch does the same using `am_offset`. A union is the exception, since it holds one member at a time, so its cursor is cleared rather than advanced.

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

### Brace elision

Brace elision is C permitting the inner braces of a nested initializer to be left out. `int e[2][2] = {1, 2, 3, 4};` initializes the same object as `{{1, 2}, {3, 4}}` without saying where each level ends.

One flag threaded through the walk is enough to handle it. `braced` records whether the current list carried braces, and `Par_FlattenSlot()` reads it to decide how to enter each slot.

An item whose value is an `AST_NODE_KIND_INITLIST` was braced in the source. `Par_Flatten()` handles it as a self-contained list, so it cannot spill back into the enclosing one.

An unbraced aggregate takes `Par_FlattenList()` with `braced` false and the same `item` cursor. Sharing that cursor is what lets a single flat list fill several levels in sequence.

Anything else is a scalar, appended by `Par_InitAt()` at the offset the caller computed. This is the only place in the whole walk where an entry is actually produced.

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

### Designator chains

A designator chain names a subobject several levels down, as `.b.y` and `[2].x` do. C allows arbitrarily many levels in one chain, so the walk has to handle a sequence rather than a step.

The chain fully determines the subobject before any value is placed, since every step is written out. Nothing is discovered on the way down, so a loop is enough and recursion would add nothing.

`Par_Step()` adds `am_offset` and narrows to `am_type` when `an_memname` is set. The member was found by `Par_Designate()` beforehand, so no lookup and no diagnostic happen here.

An index designator multiplies by `at_base->at_size` and narrows to `at_base` instead. Both branches leave `type` and `off` ready for the next designator, which lets the caller drive the chain with a loop.

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

### Local initializer stores

A flattened entry has to become an actual write. A global writes its entries into an image at compile time, while a local has none, so `Par_InitStore()` builds one store statement per entry.

An entry's offset counts bytes. Adding it to a typed pointer would scale it by that pointer's target size, so the store would land somewhere other than the offset the flattener computed.

`addr` therefore casts the address of the variable to `char *` before anything is added. Arithmetic on a `char *` counts single bytes, so the offset means exactly what was intended.

`at` adds the offset and casts back to a pointer to the slot's own type. The two casts bracket the arithmetic, so the entry's type decides the width of the store rather than the object's.

`slot` dereferences that pointer to give an lvalue at the computed offset. Wrapping the assignment in `AST_NODE_KIND_EXPR_STMT` leaves an ordinary statement the code generator already emits.

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

### Local zeroing

C specifies that whatever an initializer omits is zero. For an aggregate those omissions can be scattered anywhere inside the object, including in its padding, as `{.n = 5}` shows.

`Par_InitLocal()` therefore clears the local in full before writing anything. One statement covers every gap at once, whatever shape those gaps happen to have.

The first test sends a scalar with a plain initializer down the ordinary assignment path. Such a local has no gaps to fill, so none of the flattening below it applies.

The `AST_NODE_KIND_ZERO` node carries the object's size in `an_val` and heads the statement list. A per-element clear would be longer and would miss the padding entirely.

`Par_FlattenInit()` produces the entries, and the loop chains a store onto the zero statement for each. The function returns `zero`, because that node is the head of the list the caller inserts.

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
