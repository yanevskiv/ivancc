## Ast

The type representation gains a member list and a completeness flag. The scope gains three namespaces that C keeps separate from variables. The completeness flag is the one that matters most, because nearly every diagnostic in this stage traces back to whether a type has been defined.

### Type representation

An aggregate type has to record more than a scalar type does. A scalar is fully described by its size and its alignment. A structure additionally needs its tag, its members, and a record of whether those members have been seen.

`at_tag` holds the name the type was declared with. Diagnostics print it, and the tag lookup uses it to match a definition against an earlier declaration. It is null for an anonymous definition, which no later reference can name.

`at_members` is the member list in declaration order. Layout depends on that order, so nothing may reorder it. A union stores its members in the same list, all of them at offset zero.

`at_complete` records whether the member list has been seen. It separates `struct S;` from `struct S { int x; };`, which nothing else in the structure distinguishes. A size of zero is not the same thing as an undefined type.

`Ast_Member` carries what layout assigns and what diagnostics need. `am_offset` is the byte distance from the start of the enclosing aggregate. `am_line` names the source line, and `am_flexible` marks a trailing `d[]` that takes no space.

```c
struct Ast_Type {
    Ast_TypeKind at_kind;
    int          at_size;     // bytes an object of this type occupies
    int          at_align;    // address multiple an object must sit on
    Ast_Type    *at_base;     // pointee for PTR, element type for ARRAY
    int          at_len;      // element count for ARRAY
    char        *at_tag;      // tag a STRUCT or UNION was declared with, or NULL
    Ast_Member  *at_members;  // members of a STRUCT or UNION, in declaration order
    int          at_complete; // false until the member list has been seen
};

struct Ast_Member {
    Ast_Member *am_next;
    char       *am_name;
    Ast_Type   *am_type;
    int         am_offset;    // bytes from the start of the enclosing aggregate
    int         am_line;      // source line the member was declared on
    int         am_flexible;  // true for a trailing `d[]`, which takes no space
};
```

### Type completeness

A type is complete when its size is known. Every primitive is complete from the moment it is declared. An aggregate becomes complete only once its member list has been read.

The flag therefore has to be set in more places than the aggregate code. A type built by `Ast_NewPointer()` or `Ast_NewArray()` never passes through `Ast_LayoutAggregate()`. Leaving it false in those constructors would reject ordinary declarations.

`Ast_NewPointer()` sets the flag unconditionally. A pointer occupies eight bytes on this target whatever it points at. A pointer to an incomplete structure is itself complete, which is precisely what makes a linked list possible.

`Ast_NewArray()` copies the flag from its element type instead. An array's size is the element size multiplied by the length. An array of an undefined structure has no size, so it must be rejected where it is declared.

```c
type->at_complete = 1;                 // in Ast_NewPointer: a pointer always has a size
type->at_complete = base->at_complete; // in Ast_NewArray: only if the element type does
```

### Silent layout errors

Layout is the one piece of this stage where a mistake does not announce itself. The offsets come out wrong, but every member still reads back whatever was written to it. The program runs and produces the answers the programmer expects.

Only `sizeof` exposes the error, or a member reached through a pointer that something else computed. Code staying inside one compiler's view of a structure never notices the disagreement. The bug surfaces at the first call into a library.

A padded shape checked against a real compiler is the cheapest guard available. `struct Padded` below is the one to check first. Each of the usual mistakes produces a different wrong answer for its size.

`struct Padded` occupies 12 bytes. A compiler that forgets interior padding places `n` at offset 1 and reports 6. One that forgets tail padding stops counting after `d` and reports 9.

```c
struct Padded {
    char c;   /* offset 0, then 3 bytes of padding */
    int  n;   /* offset 4 */
    char d;   /* offset 8, then 3 bytes of tail padding */
};
```

### Member layout

`Ast_LayoutAggregate()` assigns every member an offset and gives the type its size and alignment. It runs once, from the closing action of the specifier that defined the type. It is the last step that makes the type usable.

A structure walks its members in declaration order. The running offset is rounded up to each member's own alignment before that member is placed. `Ast_AlignTo()` performs the rounding, and the offset then advances by the member's size.

The offset left after the last member is not yet the size. The total is rounded up to the widest alignment seen among the members. That final rounding is the tail padding, which keeps every element of an array of the structure aligned.

A union takes a branch of its own. Every member is placed at offset zero, and the running offset tracks the largest member rather than the sum of them. The same final rounding still applies.

The `am_flexible` branch places a member and charges nothing for it. The offset is computed but the running offset does not advance. The alignment still counts, which is why `struct Wide` comes out at 4 rather than 1.

The same walk rejects what cannot be laid out. A flexible member is rejected in a union, before the last position, or with no member ahead of it. An incomplete member type and a duplicate member name are caught in the same loop.

```c
// Place every member of an aggregate and give the type its size and alignment.
void Ast_LayoutAggregate(Ast_Type *type, Ast_Member *members, int line)
{
    int offset = 0;
    int align = 1;

    for (Ast_Member *member = members; member; member = member->am_next) {
        if (member->am_flexible) {
            if (type->at_kind == AST_TYPE_KIND_UNION) {
                Log_ShowErrorAt(member->am_line, "a union cannot have a flexible array member");
            }
            if (member->am_next) {
                Log_ShowErrorAt(member->am_line, "flexible array member '%s' must come last", member->am_name);
            }
            if (member == members) {
                Log_ShowErrorAt(member->am_line, "a struct needs a member before a flexible array member");
            }
            member->am_offset = Ast_AlignTo(offset, member->am_type->at_align);
            if (member->am_type->at_align > align) {
                align = member->am_type->at_align; /* an offset, but no size */
            }
            continue;
        }
        if (! member->am_type->at_complete) {
            Log_ShowErrorAt(member->am_line, "member '%s' has an incomplete type", member->am_name);
        }
        for (Ast_Member *seen = members; seen != member; seen = seen->am_next) {
            if (strcmp(seen->am_name, member->am_name) == 0) {
                Log_ShowErrorAt(member->am_line, "duplicate member '%s'", member->am_name);
            }
        }
        if (member->am_type->at_align > align) {
            align = member->am_type->at_align;
        }
        if (type->at_kind == AST_TYPE_KIND_UNION) {
            member->am_offset = 0;                 /* every member at zero */
            if (member->am_type->at_size > offset) {
                offset = member->am_type->at_size;
            }
            continue;
        }
        offset = Ast_AlignTo(offset, member->am_type->at_align);
        member->am_offset = offset;
        offset += member->am_type->at_size;
    }

    if (! members) {
        Log_ShowErrorAt(line, "an aggregate must declare at least one member");
    }

    type->at_members  = members;
    type->at_complete = 1;
    type->at_align    = align;
    type->at_size     = Ast_AlignTo(offset, align); /* tail padding */
}
```

### Completion order

`at_complete` is set at the end of `Ast_LayoutAggregate()`, after the member walk has finished. The type object already exists at that point, and its tag is already bound. Only the flag is outstanding.

Setting it at the start is an easy mistake to make. The tag was bound before the members were read, so the type looks ready long before it is. Nothing in the surrounding code obviously depends on the ordering.

The ordering is what catches a structure containing itself by value. `struct S` stays incomplete while its own member list is being laid out, so the member type check rejects `inner`. An early flag would let that member through.

The declaration below has to be rejected. With the flag set early, `inner` would be sized at zero and contribute nothing. `struct S` would compile into four bytes and behave as though the member were absent.

```c
struct S {
    int x;
    struct S inner; /* rejected: `struct S` has no size at this point */
};
```

### Scope namespaces

A scope now holds four lists rather than one. Variables keep the list they already had. Tags, typedef names, and enumeration constants each gain a list of their own, chained through `Ast_Scope`.

C keeps tags in a namespace separate from ordinary identifiers. `struct stat` and a variable named `stat` coexist in one program, and system headers rely on it. Merging the two lists would compile most code correctly and fail on exactly those headers.

Typedef names and enumeration constants do share the identifier namespace with variables in C. Separating them here is a deviation from the standard. It keeps every lookup simple, at the price of accepting a few programs that C rejects.

Each list is chained through the scope rather than through a global. Popping a scope restores `as_parent` and discards all four lists at once. Nothing has to be unwound entry by entry.

```c
struct Ast_Scope {
    Ast_Scope     *as_parent;   // the scope this one is nested in
    Ast_Var       *as_vars;     // declared here, innermost names first
    Ast_Tag       *as_tags;     // struct, union and enum tags declared here
    Ast_Typedef   *as_typedefs; // typedef names declared here
    Ast_EnumConst *as_enums;    // enumeration constants declared here
};
```

### Namespace operations

Each namespace needs the same pair of operations. One looks a name up, walking outwards from the innermost scope. The other declares a name, adding it to the innermost scope alone.

`Ast_FindTag()` is the outward-walking lookup. A tag declared at file scope stays visible inside every function below it. A reference finds the nearest binding, which is what gives shadowing its meaning.

`Ast_FindTagHere()` searches the innermost scope alone and exists for one caller. `Par_BeginAggregate()` uses it to tell completing a declaration from shadowing it. No other namespace needs that distinction, so no other namespace has the function.

`Ast_DeclareTag()` adds to the innermost scope without checking for a duplicate. The check belongs to the caller, which knows whether a rebinding is a redefinition or a shadow. Typedef names and enumeration constants get the same pair with the same shape.

None of this needs to be fast. A linked list per scope is enough for a compiler of this size. Replacing it with a hash table later would change no caller, because the interface says nothing about storage.

```c
Ast_Type *Ast_FindTag(const char *name);     // outwards from the innermost scope
Ast_Type *Ast_FindTagHere(const char *name); // the innermost scope alone
void      Ast_DeclareTag(const char *name, Ast_Type *type);
```

### Enum constant lookup

`Ast_FindEnumConst()` looks a name up among the enumeration constants in scope. It differs from the other lookups in its signature rather than in its behaviour. The difference exists because of what an enumerator can be.

The other lookups report failure by returning a null pointer. An enumerator is a `long` rather than a pointer, so the same trick would need a reserved value. Zero is a perfectly good enumerator, and the first one of every enum has it.

A found flag is returned instead, and the value is passed out through a pointer. The caller folds the constant only when that flag is set. A name that is not an enumerator leaves `*value` untouched.

```c
// True if name is an enumeration constant, whose value it writes to *value.
int Ast_FindEnumConst(const char *name, long *value);
```

### File scope

The outermost scope holds everything a translation unit declares at file scope. Tags, typedefs, enumeration constants, and globals all live in it. It has to outlive every function definition in the file.

A scope stack that starts empty at each function cannot provide that. A tag declared between two definitions would have nowhere to live. Such declarations are ordinary C and appear in every header a program includes.

`Ast_FileScope` is a fixed object rather than a pointer that can be null. `Ast_CurScope` starts out pointing at it and returns to it whenever a definition closes. Nothing declared at file scope is ever discarded.

`Ast_BeginScope()` resets `Ast_CurScope` to the file scope before pushing a fresh one. The pushed scope is what holds parameters and locals. `Ast_Locals` is cleared at the same time, since stack offsets are assigned per function.

`Ast_EndScope()` restores the file scope rather than walking the chain back. Every scope the function pushed is abandoned in one assignment. Parameters and locals disappear, while everything at file scope survives.

```c
static Ast_Scope Ast_FileScope;                   /* outlives every function */
static Ast_Scope *Ast_CurScope = &Ast_FileScope;

void Ast_BeginScope(void)
{
    Ast_Locals   = NULL;
    Ast_CurScope = &Ast_FileScope;
    Ast_PushScope();
}

void Ast_EndScope(void)
{
    Ast_CurScope = &Ast_FileScope;                /* not NULL */
}
```

### Node kinds

This stage adds five node kinds to the tree. One of them is for member access. The remaining four exist to carry initializers from the grammar through to the code generator.

`AST_NODE_KIND_MEMBER` holds the name as written and the member it resolves to. Both fields are needed, because the grammar can fill only the first. `a->b` reaches this kind as a dereference with a member access above it.

`AST_NODE_KIND_INITLIST` is a braced list, with its items chained on `an_body`. `AST_NODE_KIND_DESIGNATOR` is a single `[n]` or `.name`. Both are transcription, discarded once the flattening walk has read them.

`AST_NODE_KIND_INIT` is one flattened entry. Its `an_val` holds a byte offset into the object rather than a number the program wrote. That is the one field in the tree whose meaning differs from its name, which is why it carries a comment.

`AST_NODE_KIND_ZERO` clears `an_val` bytes of the object that `an_lhs` addresses. It is a statement rather than an expression. Nothing in the source produces it, since only local initialization emits it.

```c
    AST_NODE_KIND_MEMBER,     // lhs.an_member, with `a->b` parsed as `(*a).b`
    AST_NODE_KIND_INIT,       // one flattened initializer: an_val is a byte offset
    AST_NODE_KIND_INITLIST,   // a braced initializer list, its items chained on an_body
    AST_NODE_KIND_DESIGNATOR, // `[an_val]` or `.an_memname` naming where an item lands
    AST_NODE_KIND_ZERO,       // zero an_val bytes of the object an_lhs addresses
```
