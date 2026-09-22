## Parser

Parser work divides into four largely independent pieces. They are declarations that name only a type, the aggregate and enum specifiers themselves, member access, and a rebuilt initializer syntax. The initializer work is by far the largest of the four.

### Type-only declarations

A type-only declaration names a type and declares no object. `struct S { int x; };` is one such declaration. So is `enum Color { RED };`, which introduces three constants but no variable.

A rule written as `storage type_name IDENT ...` cannot express either of them. The rule requires a declarator before the semicolon. Every declaration reaching it must name an object.

Factoring the common prefix out resolves this. `external_decl` reduces the storage class and the type specifier alone. What follows is left to `external_tail`, which accepts either a semicolon or a declarator.

The mid-rule action in `external_decl` stores the storage class in `Par_DeclStorage` and the type in `Par_DeclType`. The declarator actions run later and need both of them. By then the parser has left the stack positions where the prefix appeared.

The `SEMI` alternative of `external_tail` ends the declaration with no declarator at all. The `IDENT` alternative records the name in `Par_DeclName` before continuing. `decl_tail` then handles pointers, arrays, and any initializer.

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

A block may contain a type-only declaration exactly as a translation unit may. `struct S { int x; };` inside a function body is ordinary C. The local declaration rules therefore need the same factoring as the file-scope ones.

`decl` mirrors `external_decl`. Its mid-rule action fills `Par_DeclType` and `Par_DeclStorage` once the shared prefix has reduced. The two rules stay separate because a local declaration produces a statement list for its block, while a file-scope one produces nothing.

The bare case cannot be split into a second rule for `decl`. A mid-rule action is itself a reduction, so bison must choose between two rules sharing a prefix before that action runs. The choice is not determined at that point.

`decl_body` expresses the bare case as an empty alternative instead. The mid-rule action then belongs to one rule only. The parser reduces it unconditionally and decides afterwards whether a declarator list follows.

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

A struct or union specifier is whatever names an aggregate type in a declaration. Three forms exist: a definition carrying a tag, an anonymous definition, and a bare reference to a tag. All three are type specifiers, so all three belong in `base` beside `int`, `char`, and `void`.

A structure may contain a pointer to its own type. `struct node { struct node *next; };` is ordinary C. The member list therefore refers to a type that the specifier producing it has not finished building.

The two defining alternatives of `base` split the work across two actions. A mid-rule action calls `Par_BeginAggregate()` at the opening brace. The tag is bound before `members` is parsed, so the reference inside resolves.

`Ast_LayoutAggregate()` runs in the closing action. It takes the type back out of `$<type>` and receives the finished member list. A single action after `RBRACE` could not work, because the type would not exist while the members were being read.

The third alternative calls `Par_ReferenceAggregate()` and names no member list. It covers `struct node *p;` and a forward declaration alike. The `TYPEDEF_NAME` alternative sits beside it and resolves an already bound name through `Ast_FindTypedef()`.

`struct_or_union` collapses two keywords into one value. It yields `AST_TYPE_KIND_STRUCT` or `AST_TYPE_KIND_UNION`. Every alternative above takes that kind as a parameter, so none of them is written out twice.

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

An aggregate definition is a specifier carrying a member list, such as `struct Point { int x; int y; };`. Reading one creates a type, binds its tag to that type, and lays out its members. `Par_BeginAggregate()` is responsible for the first two of those steps.

The tag may already be bound by the time the definition is read. Three things can have bound it: a forward declaration such as `struct Point;`, a definition in an enclosing scope, or an earlier definition in this same scope.

`Ast_FindTagHere()` tells the second case from the other two. It searches the innermost scope alone, so a binding inherited from an enclosing scope is invisible to it. The definition then creates a fresh type that shadows the outer one, which is what C requires.

The `at_complete` flag tells the first case from the third. A tag bound to an incomplete type came from a forward declaration, so that type is reused and completed here. A tag bound to a complete type came from an earlier definition, which makes this one a redefinition.

The kind test guards against `union Point` following `struct Point`. Such a pair would otherwise share a single tag. The resulting type would carry whichever layout the parser read first, with no diagnostic to show for it.

`Ast_NewAggregate()` creates a type when no existing binding can be reused. `Ast_DeclareTag()` then binds the tag to it. An anonymous definition skips that binding, since a null `tag` leaves nothing for a later reference to name.

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

A tag reference is a specifier that names an aggregate without defining it. `struct node` in the declaration `struct node *p;` is one example. `Par_ReferenceAggregate()` turns such a reference into a type.

The tag named may never have been declared anywhere. A program may write `struct list *head;` before `struct list` exists. Rejecting that would break forward declarations and self-referential structures alike.

A reference to an unknown tag therefore creates an incomplete type rather than an error. `Ast_NewAggregate()` builds it and `Ast_DeclareTag()` binds it immediately. A pointer to an incomplete type has a known size, so the declaration can still be laid out.

`Ast_FindTag()` searches every enclosing scope rather than the innermost alone. A tag declared at file scope has to stay visible inside a function. This is the one difference between this lookup and the one a definition performs.

The kind test repeats the one in `Par_BeginAggregate()`. A reference can disagree with a definition just as two definitions can disagree. `union node *p;` after `struct node { ... };` is caught here rather than at the member access that would follow.

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

Tags and ordinary identifiers occupy separate namespaces in C. A tag may therefore reuse a name that a typedef has already bound. The declaration `typedef struct node node;` does exactly that, and it is a common idiom.

The lexer's typedef feedback does not respect that separation. Once `node` is bound, every later occurrence of it arrives as `TYPEDEF_NAME`. A tag position expecting `IDENT` will not accept the token.

`tag_name` accepts either token and yields the same string either way. The actions above it never learn which one arrived. `struct node` continues to work after the typedef because of this one rule.

Leaving the `TYPEDEF_NAME` alternative out produces a syntax error pointing at the tag. That diagnostic is misleading. The declaration it complains about is correct C, and the real fault lies in the lexer two rules away.

```c
tag_name
    : IDENT                { $$ = $1; }
    | TYPEDEF_NAME         { $$ = $1; } /* `typedef struct node node;` */
    ;
```

### Member declarations

A member declaration is one line of an aggregate's member list. It is a type specifier followed by one or more declarators. `int x, *p, a[4];` is a single member declaration that declares three members.

Those three declarators share one type specifier and produce three different types. A declarator cannot therefore build its type as it reduces. The shared type has to reach each of them separately, once all of them have been seen.

`members` accumulates the declarations of one aggregate. Its empty alternative lets the grammar reach `Ast_LayoutAggregate()` with a null list, which is how an empty aggregate is diagnosed rather than crashed on. `Par_AppendMembers()` joins each new group onto the end, preserving declaration order.

`member_decl` is one declaration: a `type_name`, its declarators, and a semicolon. It builds nothing itself. Both parts go to `Par_MakeMembers()`, which is where the shared type is distributed.

`member_declarator` collects a name and its dimensions without building a type. Its first alternative carries `array_dims` through `an_lhs`. Its second matches the empty brackets of a flexible array member and records that fact in `an_val`.

Both alternatives build an `AST_NODE_KIND_NOP` node. The node is a carrier for a name and a dimension list, nothing more. Nothing downstream evaluates it, and it is discarded once `Par_MakeMembers()` has read it.

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

`Par_MakeMembers()` turns the collected declarators into a member list. It runs once per member declaration, with the shared type passed in as an argument. Each declarator contributes exactly one member to the result.

`Par_ArrayType()` applies each declarator's dimensions to that shared type. It takes the type and the `an_lhs` list the declarator carried. It returns a new type rather than modifying the old one, so the next declarator still sees the shared type unchanged.

The `an_val` test picks out a flexible array member. Such a declarator was given no dimensions at all. Its type is replaced with `Ast_NewArray(type, 0)`, and `am_flexible` is set for the layout pass to act on.

The `head` and `tail` pair builds the list in order without a special case for the first member. `head` is a dummy whose `am_next` becomes the result. That is why the function returns `head.am_next` rather than a variable of its own.

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

A member declaration may itself define an aggregate. A structure definition is a type specifier, and a type specifier is what a member declaration begins with. One member list can therefore open while another is still being read.

The example below defines `struct B` inside `struct A`'s member list. The inner `member_decl` reduces while the outer `members` is still accumulating. `y` follows that inner definition and must still come out as an `int`.

A single "type of the declaration being parsed" global would be overwritten as `struct B` reduced. `y` would come out as a `struct B` instead. The size of `struct A` would then be wrong, with no diagnostic anywhere to say so.

Passing the type to `Par_MakeMembers()` as an argument avoids the problem entirely. Each member declaration carries its own type down the parser stack. Nesting costs nothing, because the two lists share no state.

```c
struct A {
    struct B { int x; } b; /* opens and closes a second member list */
    int y;                 /* must still be int, not struct B */
};
```

### Enum specifiers

An enum specifier declares a set of named integer constants. `enum Color { RED, GREEN, BLUE };` declares three of them. The specifier also yields a type, for declarations such as `enum Color c;`.

That type needs no kind of its own. C permits the implementation to choose any compatible integer type. `int` is the simplest honest choice, so every alternative yields `&Ast_TypeInt`.

The tagged alternative calls `Ast_DeclareTag()` with `&Ast_TypeInt`. A later `enum Color c;` then resolves through the ordinary tag lookup. Binding the tag to a primitive is why nothing else in the compiler has to know that enums exist.

The mid-rule action resetting `Par_EnumValue` runs at the opening brace. It runs before `enumerators` is parsed. Each enum therefore starts counting from zero, whatever the previous one left in the variable.

The third alternative is `ENUM tag_name` with no braces. It yields `&Ast_TypeInt` without consulting the tag at all. A reference to an undeclared enum tag is accepted as `int` rather than reported, which is a deliberate simplification here.

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

An enumerator is one name within an enum specifier's braces. It is declared at the current value of a running counter. `Par_AddEnumConst()` performs that declaration and then steps the counter by one.

An enumerator may carry an explicit value, as `FAILED = 20` does. That value becomes the new running value rather than applying to one name alone. The enumerators after it continue counting from there.

An explicit value must be a constant expression in C. This stage has no constant folder. Nothing is available to evaluate `= 1 + 1` or `= sizeof(int)` at the point where the enumerator reduces.

The `AST_NODE_KIND_NUM` test rejects anything the parser has not already reduced to a single number. `= 10` is admitted. Everything else is reported against the enumerator by name, rather than surfacing later as a wrong value.

`Ast_DeclareEnumConst()` receives `Par_EnumValue` and the post-increment steps it within the same expression. The constant lands in the scope rather than in any type. That is where the expression rules will go looking for it.

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

An enumeration constant is a value rather than an object. The name `RED` stands for the number 0 and for nothing else. No storage was ever allocated for it, so there is no address to load from.

The grammar reads `RED` through the same rule that reads a variable. Left alone, that rule calls `Ast_FindVar()` and reports an undeclared identifier. The diagnostic would point at a correct use of a correctly declared constant.

`Ast_FindEnumConst()` is therefore consulted first, before `Ast_FindVar()` runs at all. A name it recognises is replaced by `Ast_NewNum()`. The resulting node is indistinguishable from a literal for everything downstream.

The `else` branch is the ordinary variable path, unchanged except that it now runs second. A name that is neither an enumerator nor a variable still reports as undeclared. The added cost is one failed search.

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

An array bound is the other place an enumeration constant is commonly used. `int table[GONE];` sizes an array from an enumerator. Naming a bound this way is most of the reason to declare the constant at all.

An array dimension rule that accepts only `NUM` rejects that declaration. The fold in the expression rules does not help here. A dimension is read by a different rule, which never reaches `primary`.

`array_len` accepts both forms in a rule of its own. `NUM` stays as the first alternative, so `int a[4];` reduces exactly as it did before. Nothing that already worked changes shape.

The `IDENT` alternative folds through `Ast_FindEnumConst()` and rejects anything it does not recognise. The value is produced as a number. The rules above `array_len` continue to receive a `long` and need no changes of their own.

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

A typedef declaration has the same shape as an object declaration. `typedef char *String;` and `char *s;` differ by one keyword. Only what the declaration means at the end differs.

Treating `typedef` as a storage class exploits that shape. `storage` gains a `TYPEDEF` alternative beside `STATIC` and `EXTERN`, yielding `AST_STORAGE_TYPEDEF`. The declarator rules are then reached without any change of their own.

Pointers and arrays come along for free. The declarator already handles both of them for objects. `typedef int Row[4];` reduces through the same path as `int row[4];`, differing only in the value `storage` produced.

The empty alternative of `storage` stays where it is. A declaration with no storage class at all still reduces through the same rule. Adding `TYPEDEF` introduces no conflict, because the alternatives are distinguished by their first token.

```c
storage
    : /* empty */          { $$ = AST_STORAGE_NONE; }
    | STATIC               { $$ = AST_STORAGE_STATIC; }
    | EXTERN               { $$ = AST_STORAGE_EXTERN; }
    | TYPEDEF              { $$ = AST_STORAGE_TYPEDEF; }
    ;
```

### Typedef name binding

`Par_AddGlobal()` is where a declarator's name becomes a declaration. It runs once per declarator, after that declarator has reduced. What it declares depends on the storage class that `storage` recorded.

Under `AST_STORAGE_TYPEDEF` it binds the name as a type through `Ast_DeclareTypedef()`. No object is created and no storage is reserved. The early return leaves the rest of the function untouched.

`Par_ArrayType()` runs before that binding, so everything the declarator built goes into the bound type. `typedef int Row[4];` binds `Row` to a four-element array. Dropping the call would bind `Row` to `int` and lose the dimensions silently.

Registration happens here, during the reduction of each declarator. It does not wait for the end of the declaration. That timing is what makes the lexer's typedef feedback work for `typedef int Foo, Bar;`.

The rule that declares a local needs the same branch. A typedef may appear inside a function body, where `Par_AddGlobal()` never runs. `Par_DeclareLocal()` tests the same storage class and calls the same binding function.

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

`.` and `->` are the two ways of reaching a member. `p.x` reads a member out of an object directly. `p->x` reads one through a pointer to that object.

Both belong at the postfix level. They bind more tightly than any prefix operator, so `&p.x` takes the address of the member rather than of `p`. They chain from left to right, so `a.b.c` parses as `(a.b).c`.

The `DOT` alternative passes its left operand straight to `Ast_NewMemberNode()`. Left recursion on `postfix` is what produces the chaining. The result of one member access is itself a `postfix` for the next.

The `ARROW` alternative wraps the operand in `AST_NODE_KIND_DEREF` first. C defines `a->b` as `(*a).b`, so the grammar can build that form directly. The semantic pass, the lvalue test, and the code generator then handle one node kind instead of two.

```c
postfix
    : /* ... calls, subscripts, postfix ++ and -- ... */
    | postfix DOT IDENT    { $$ = Ast_NewMemberNode($1, $3, @2); }
    | postfix ARROW IDENT  /* built as `(*a).b` */
        { $$ = Ast_NewMemberNode(Ast_NewUnary(AST_NODE_KIND_DEREF, $1, @2), $3, @2); }
    ;
```

### Deferred member resolution

`Ast_NewMemberNode()` builds the node that a member access reduces to. It receives the left operand and the name written after the operator. What it does not do is find the member.

Finding the member requires the type of the left operand. That type is not known while the expression is reducing. `p->x` gives the parser an expression and a name with nothing to connect them.

The name is therefore stored and the resolution deferred. `Ast_NewUnary()` builds the node with `AST_NODE_KIND_MEMBER` and the operand as its left child. The member pointer stays null until the semantic pass fills it in.

`strdup()` copies the name into `an_memname` rather than keeping the lexer's pointer. The lexer reuses its buffer for the next token. Without the copy, the node would name whatever token happened to follow.

The semantic pass walks the tree from the bottom up. The left operand therefore carries a type by the time the member node is reached. Nothing between the two passes reads the null member pointer.

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

An initializer for an aggregate can take several forms at once. It can nest, as `{{1, 2}, {3, 4}}` does. It can omit braces, as `{1, 2, 3, 4}` does for that same type. It can aim an item at a subobject, as `{.b.y = 9}` does.

Element indices cannot express all three together. A designator can name a member of a member. An elided brace can cross a nesting level in the middle of a list.

Flattening everything to byte offsets removes the difficulty. Every item becomes an offset, a type, and a value. A global and a local can then share one computation, because a byte offset means the same thing to both.

The grammar records what was written and interprets none of it. Resolution needs the type being initialized, which is not available where these rules reduce. Everything below is a faithful transcription rather than a decision.

`initializer` is either an expression or a braced list. The braced case builds an `AST_NODE_KIND_INITLIST` node of its own. That node is what later distinguishes a nested list from a scalar, which a flat run of items could not do.

`init_item` wraps each item in `AST_NODE_KIND_INIT` and hangs any designators on `an_cond`. The field is borrowed rather than added. An initializer item has no condition of its own to keep there.

`designator` records `[4]` in `an_val` and `.y` in `an_memname`. It does nothing further with either. A chain of them is left as a list for the flattening walk to resolve.

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

`Par_FlattenList()` is the walk that turns a recorded initializer into entries. It visits the type being initialized alongside the list that was written. It produces one entry per scalar, carrying a byte offset, a slot type, and a value.

The walk carries a cursor over the type. `index` tracks position within an array and `member` tracks position within a structure. Exactly one of the two is meaningful at any moment, decided by the type's kind.

The `an_cond` branch resolves a designator chain before placing anything. `Par_Designate()` moves the cursor to the named subobject. `Par_Step()` accumulates the offset, looping over `an_next` for each further designator in the chain.

The array branch places an item at `index * at_size` from the base and then steps the index. An index past `at_len` means different things in the two cases. It is an error in a braced list and a signal to return in an elided one.

The member branch does the same using `am_offset`, which the layout pass has already computed. A union is the exception. Its cursor is cleared rather than stepped, so a second item is reported instead of overwriting the first.

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

Brace elision is C permitting the inner braces of a nested initializer to be left out. `int e[2][2] = {1, 2, 3, 4};` initializes the same object as `{{1, 2}, {3, 4}}`. Both forms have to produce the same entries.

One flag threaded through the walk is enough to handle it. `braced` records whether the list currently being filled carried braces in the source. `Par_FlattenSlot()` reads that flag to decide how to enter each slot.

An item whose value is an `AST_NODE_KIND_INITLIST` was braced in the source. `Par_Flatten()` handles it and the cursor advances past it. The nested list is complete in itself and cannot spill back into the enclosing one.

An unbraced aggregate or array takes `Par_FlattenList()` with `braced` false and the same `item` cursor. The inner object consumes items until it is full. It then returns, leaving whatever remains to the enclosing list.

Anything else is a scalar. `Par_InitAt()` appends it at the offset the caller computed. This is the only place in the whole walk where an entry is actually produced.

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

A designator chain names a subobject several levels below the object being initialized. `.b.y` reaches a member of a member. `[2].x` reaches a member of an array element.

The chain is resolved whole rather than descended through recursively. It fully determines the subobject it names before any value is placed. Nothing is discovered on the way down, so a loop is enough.

`Par_Step()` adds `am_offset` and narrows the type to `am_type` when `an_memname` is set. The member itself was found by `Par_Designate()` before this call. No lookup happens here, and no diagnostic can come out of it.

An index designator multiplies by `at_base->at_size` and narrows to `at_base` instead. Both branches leave `type` and `off` ready for the next designator. That is what lets the caller drive the chain with a plain loop.

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

A flattened entry has to become an actual write. A global writes its entries into an image as bytes at compile time. A local has no image, so `Par_InitStore()` builds one store statement per entry instead.

An entry's offset is a count of bytes. Adding it to a typed pointer would scale it by that pointer's target size. The store would land somewhere other than the offset the flattener computed.

`addr` casts the address of the variable to `char *` before anything is added to it. Arithmetic on a `char *` counts single bytes. The offset therefore means exactly what the flattener intended.

`at` adds the byte offset and casts the result back to a pointer to the slot's own type. The two casts bracket the arithmetic. The entry's type decides the width of the store, and the object's type has no say in it.

`slot` dereferences that pointer to give an lvalue at exactly the computed offset. The assignment is wrapped in `AST_NODE_KIND_EXPR_STMT`. The result is an ordinary statement that the code generator already knows how to emit.

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

C specifies that whatever an initializer omits is zero. For a scalar that means one value. For an aggregate the omissions can be scattered anywhere inside the object, including in its padding.

`Par_InitLocal()` therefore clears the local in full before writing anything to it. One statement covers every gap at once. Writing the entries afterwards leaves the untouched bytes at zero with no further bookkeeping.

The first test sends a scalar with a plain initializer down the ordinary assignment path. Such a local has no gaps to fill and no offsets to compute. None of the flattening below it applies.

The `AST_NODE_KIND_ZERO` node carries the object's size in `an_val` and heads the statement list. One statement clearing everything is shorter than a store per element. It is also the only way `{.n = 5}` leaves the other members and the padding bytes at zero.

`Par_FlattenInit()` produces the entries and the loop turns each into a store through `Par_InitStore()`. Each store is chained onto the zero statement in turn. The function returns `zero`, because that node is the head of the list the caller inserts.

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
