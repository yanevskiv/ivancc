## Ast

The type representation gains a member list and a completeness flag. The scope gains three namespaces that C keeps separate from variables, along with an outermost scope that survives each function definition. The completeness flag is the one that matters most, because nearly every diagnostic in this stage traces back to it.

### Type representation

An aggregate type has to record more than a scalar type does. A scalar is fully described by its size and alignment, while a structure also needs its tag, its members, and a record of whether those members have been seen.

`at_tag` holds the name the type was declared with. Diagnostics print it, and the tag lookup uses it to match a definition against an earlier declaration. It is null for an anonymous definition, which nothing can refer to by name.

`at_members` is the member list in declaration order, which layout depends on and nothing may reorder. A union stores its members in the same list, all at offset zero, so every walk over the members stays identical.

`at_complete` records whether the member list has been seen. Nothing else in the structure separates `struct S;` from `struct S { int x; };`, since a size of zero is not the same as an undefined type.

`Ast_Member` carries what layout assigns and what diagnostics need. `am_offset` is the byte distance the code generator adds to an address, `am_line` names the source line, and `am_flexible` marks a trailing `d[]`.

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

A type is complete when its size is known. Every primitive is complete from the moment it is declared, while an aggregate becomes complete only once its member list has been read and laid out.

The flag therefore has to be set in more places than the aggregate code. A type built by `Ast_NewPointer()` or `Ast_NewArray()` never reaches `Ast_LayoutAggregate()`, so leaving it false there would reject `int *p;`.

`Ast_NewPointer()` sets the flag unconditionally, because a pointer occupies eight bytes whatever it points at. A pointer to an incomplete structure is consequently complete, which is what makes a linked list possible.

`Ast_NewArray()` copies the flag from its element type instead. An array's size is the element size multiplied by the length, so an array of an undefined structure has no size and must be rejected.

```c
type->at_complete = 1;                 // in Ast_NewPointer: a pointer always has a size
type->at_complete = base->at_complete; // in Ast_NewArray: only if the element type does
```

### Silent layout errors

Layout is the one piece of this stage where a mistake does not announce itself. The offsets come out wrong, yet every member still reads back whatever was written to it, because the same wrong offsets are used everywhere.

Only `sizeof` exposes the error, or a member reached through a pointer that something else computed. The bug therefore surfaces at the first call into a library, a long way from the code that caused it.

A padded shape checked against a real compiler is the cheapest guard available. Each of the usual mistakes produces a different wrong answer, so one number narrows the fault rather than merely detecting it.

`struct Padded` occupies 12 bytes. Forgetting interior padding places `n` at offset 1 and gives 6, while forgetting tail padding stops after `d` and gives 9, which leaves only the final rounding missing.

```c
struct Padded {
    char c;   /* offset 0, then 3 bytes of padding */
    int  n;   /* offset 4 */
    char d;   /* offset 8, then 3 bytes of tail padding */
};
```

### Member layout

`Ast_LayoutAggregate()` assigns every member an offset and gives the type its size and alignment. It runs once, from the closing action of the specifier that defined the type, and is the last step that makes the type usable.

A structure walks its members in declaration order, rounding the running offset up to each member's own alignment before placing it. A `char` at offset 0 followed by an `int` therefore leaves three bytes unused.

The offset left after the last member is not yet the size. The total is rounded up to the widest alignment seen, which is the tail padding that keeps every element of an array of the structure aligned.

A union places every member at offset zero and tracks the largest rather than the sum. The same final rounding still applies, which is why `union Mixed` comes out at 8 rather than at 13.

The `am_flexible` branch computes an offset without advancing the running offset. The alignment still counts, however, which is why `struct Wide` comes out at 4 rather than at 1.

The same walk rejects what cannot be laid out. A flexible member is rejected in a union, before the last position, or with no member ahead of it, and duplicates and incomplete types are caught in the same loop.

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

`at_complete` is set at the end of the function, after the member walk has finished. The type already exists and its tag was bound before the members were read, so only the flag is outstanding.

Setting it at the start is an easy mistake to make. The tag was bound early so that a member could point at the type, which makes the type look ready long before it actually is.

The ordering is what catches a structure containing itself by value. `struct S` stays incomplete while its own members are laid out, so the member type check rejects `inner` rather than accepting it.

The declaration below has to be rejected. With the flag set early, `inner` would be sized at zero, `struct S` would compile into four bytes, and every read of `inner` would alias `x`.

```c
struct S {
    int x;
    struct S inner; /* rejected: `struct S` has no size at this point */
};
```

### Scope namespaces

A scope now holds four lists rather than one. Variables keep the list they had, while tags, typedef names, and enumeration constants each gain one of their own, chained through `Ast_Scope`.

C keeps tags in a namespace separate from ordinary identifiers, which is why `struct stat` and a variable named `stat` coexist. Merging the lists would compile most code correctly and fail on exactly the headers that matter.

Typedef names and enumeration constants do share the identifier namespace with variables in C. Separating them here is a deviation, and the price is accepting a few programs that C rejects.

Each list is chained through the scope rather than through a global. Popping a scope restores `as_parent` and discards all four at once, so nothing has to be unwound entry by entry.

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

Each namespace needs the same pair of operations. One looks a name up, walking outwards from the innermost scope, and the other declares a name in the innermost scope alone.

`Ast_FindTag()` is the outward-walking lookup, which is what keeps a file-scope tag visible inside every function. It finds the nearest binding rather than the first declared, which is what gives shadowing its meaning.

`Ast_FindTagHere()` searches the innermost scope alone and exists for one caller. `Par_BeginAggregate()` uses it to tell completing a declaration from shadowing it, which the outward walk cannot distinguish.

`Ast_DeclareTag()` adds to the innermost scope without checking for a duplicate. The check belongs to the caller, which is the only place that knows whether a rebinding is a redefinition or a shadow.

```c
Ast_Type *Ast_FindTag(const char *name);     // outwards from the innermost scope
Ast_Type *Ast_FindTagHere(const char *name); // the innermost scope alone
void      Ast_DeclareTag(const char *name, Ast_Type *type);
```

### Enum constant lookup

`Ast_FindEnumConst()` looks a name up among the enumeration constants in scope. It walks outwards exactly as `Ast_FindTag()` does, but it cannot report its answer the same way.

The other lookups report failure by returning a null pointer. An enumerator is a `long`, so the same trick needs a reserved value, and zero is a perfectly good enumerator that every enum's first constant has.

A found flag is returned instead, and the value is passed out through a pointer. The caller folds the constant only when that flag is set, falling through to the variable lookup otherwise.

```c
// True if name is an enumeration constant, whose value it writes to *value.
int Ast_FindEnumConst(const char *name, long *value);
```

### File scope

The outermost scope holds everything a translation unit declares at file scope. It has to outlive every function definition, since a type declared before one function must stay visible to the next.

A scope stack that starts empty at each function cannot provide that. A tag declared between two definitions would have nowhere to live, and such declarations appear in every header a program includes.

`Ast_FileScope` is therefore a fixed object rather than a pointer that can be null. `Ast_CurScope` starts out pointing at it and returns to it whenever a definition closes, so there is always a scope to declare into.

`Ast_BeginScope()` resets `Ast_CurScope` before pushing a fresh scope, because the previous definition may have left the pointer elsewhere. `Ast_Locals` is cleared at the same time, since offsets are assigned per function.

`Ast_EndScope()` restores the file scope rather than walking the chain back. Every scope the function pushed is abandoned in one assignment, however deeply the blocks were nested.

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

This stage adds five node kinds. One is for member access, and the remaining four exist to carry initializers from the grammar through to the code generator.

`AST_NODE_KIND_MEMBER` holds the name as written and the member it resolves to. Both fields are needed, because the grammar can fill only the first and the generator can use only the second.

`AST_NODE_KIND_INITLIST` is a braced list and `AST_NODE_KIND_DESIGNATOR` is a single `[n]` or `.name`. Both are transcription, discarded once the flattening walk has read them, so neither reaches the semantic pass.

`AST_NODE_KIND_INIT` is one flattened entry. Its `an_val` holds a byte offset rather than a number the program wrote, which is the one field in the tree whose meaning differs from its name.

`AST_NODE_KIND_ZERO` clears `an_val` bytes of the object that `an_lhs` addresses. It is a statement rather than an expression, and nothing in the source produces it, since only `Par_InitLocal()` emits it.

```c
    AST_NODE_KIND_MEMBER,     // lhs.an_member, with `a->b` parsed as `(*a).b`
    AST_NODE_KIND_INIT,       // one flattened initializer: an_val is a byte offset
    AST_NODE_KIND_INITLIST,   // a braced initializer list, its items chained on an_body
    AST_NODE_KIND_DESIGNATOR, // `[an_val]` or `.an_memname` naming where an item lands
    AST_NODE_KIND_ZERO,       // zero an_val bytes of the object an_lhs addresses
```
