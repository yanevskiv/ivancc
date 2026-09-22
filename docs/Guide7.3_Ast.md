## Ast

The scalar type model has no place for aggregate tags, members, completeness, or initializer nodes, and the scope has no separate tag state. The AST adds those records, file scope, and member and initializer node kinds so later passes can preserve what the parser found. This keeps layout data and source names available across functions, while semantic checks decide whether each use is valid.

### Extend: `Ast_Type`

`Ast_Type` described a type by its kind, its size, its alignment and what it points at. Three fields join the structure: `at_tag` for diagnostics and tag matching, `at_members` for the list, and `at_complete` for whether that list has been read. A type of size zero and a type whose members were never seen are different things, and only the flag separates `struct S;` from `struct S { int x; };`.

```c
struct Ast_Type {

    char        *at_tag;      // tag a STRUCT or UNION was declared with, or NULL
    Ast_Member  *at_members;  // members of a STRUCT or UNION, in declaration order
    int          at_complete; // false until the member list has been seen
};
```

### Add: `Ast_Member`

A member is a name, a type and a position inside the aggregate that holds it. `Ast_Member` chains through `am_next` in declaration order, which layout depends on and nothing may reorder. `am_flexible` marks a trailing `d[]`, which layout places without charging the structure for it.

```c
struct Ast_Member {
    Ast_Member *am_next;
    char       *am_name;
    Ast_Type   *am_type;
    int         am_offset;    // bytes from the start of the enclosing aggregate
    int         am_line;      // source line the member was declared on
    int         am_flexible;  // true for a trailing `d[]`, which takes no space
};
```

### Extend: `Ast_NewPointer()`

A type is complete when its size is known, which `at_complete` now records. The constructor sets it unconditionally, since a pointer occupies eight bytes on this target whatever it points at. `struct node *next;` inside `struct node` relies on that, and a stricter rule would reject the commonest use of the feature.

```c
// Build the type of a pointer to base.
Ast_Type *Ast_NewPointer(Ast_Type *base)
{

    type->at_base     = base;
    type->at_complete = 1;
    return type;
}
```

### Extend: `Ast_NewArray()`

An array's size is its element size multiplied by its length, which needs the element type to have a size. The constructor copies the flag from the element type rather than setting it. An array of arrays of an undefined structure comes out incomplete too, without the constructor knowing anything about the depth.

```c
// Build the type of an array of len elements of base.
Ast_Type *Ast_NewArray(Ast_Type *base, int len)
{

    type->at_size     = base->at_size * len;
    type->at_len      = len;
    type->at_complete = base->at_complete;
    return type;
}
```

### Add: `Ast_LayoutAggregate()`

Every member needs a byte offset, and the type needs the size and alignment an object of it occupies. `Ast_LayoutAggregate()` walks the members in order, rounding the running offset up to each member's alignment before placing it. The flag is set at the end rather than the start, which is what catches a structure containing itself by value.

```c
// Place every member of an aggregate and give the type its size and alignment.
void Ast_LayoutAggregate(Ast_Type *type, Ast_Member *members, int line)
{
    int offset = 0;
    int align = 1;

    for (Ast_Member *member = members; member; member = member->am_next) {
        if (member->am_flexible) {

            member->am_offset = Ast_AlignTo(offset, member->am_type->at_align);
            if (member->am_type->at_align > align) {
                align = member->am_type->at_align; /* an offset, but no size */
            }
            continue;
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

    type->at_members  = members;
    type->at_complete = 1;
    type->at_align    = align;
    type->at_size     = Ast_AlignTo(offset, align); /* tail padding */
}
```

### Extend: `Ast_Scope`

A scope held the variables declared in it, chained through one list, but C also separates tags from ordinary declarations and needs file scope to survive function definitions. Three further lists join the structure, one per namespace, so aggregate and enum lookups do not compete with variables. The separation keeps each lookup simple, at the price of accepting a few programs that C rejects.

```c
struct Ast_Scope {
    Ast_Scope     *as_parent;   // the scope this one is nested in
    Ast_Var       *as_vars;     // declared here, innermost names first
    Ast_Tag       *as_tags;     // struct, union and enum tags declared here
    Ast_Typedef   *as_typedefs; // typedef names declared here
    Ast_EnumConst *as_enums;    // enumeration constants declared here
};
```

### Add: `Ast_FindTagHere()`

A lookup walks outwards from the innermost scope, which is what keeps a file-scope tag visible inside a function. `Ast_FindTagHere()` searches the innermost scope alone, beside the outward-walking `Ast_FindTag()`. `Ast_DeclareTag()` adds without checking for a duplicate, because only the caller knows whether a rebinding is a redefinition.

```c
Ast_Type *Ast_FindTag(const char *name);     // outwards from the innermost scope
Ast_Type *Ast_FindTagHere(const char *name); // the innermost scope alone
void      Ast_DeclareTag(const char *name, Ast_Type *type);
```

### Add: `Ast_FindEnumConst()`

Every other lookup reports failure by returning a null pointer, which the caller tests. `Ast_FindEnumConst()` returns a found flag and passes the value out through a pointer. A name that is not an enumerator leaves `*value` untouched, so a caller ignoring the flag gets a stale value rather than a plausible one.

```c
// True if name is an enumeration constant, whose value it writes to *value.
int Ast_FindEnumConst(const char *name, long *value);
```

### Add: `Ast_FileScope`

The scope stack started empty at each function and was cleared when the definition closed. `Ast_FileScope` is a fixed object that `Ast_CurScope` points at outside any function. `Ast_EndScope()` restores the file scope in one assignment rather than walking the chain, which discards however deeply the blocks were nested.

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

### Add: `AST_NODE_KIND_MEMBER`

Member access is an expression the tree had no kind for, and an initializer is a shape it had no way to record. `AST_NODE_KIND_MEMBER` carries the name as written and the member it resolves to, because the grammar can fill only the first. `a->b` needs no kind of its own, since the grammar builds it as a dereference with a member access above it.

```c
    AST_NODE_KIND_MEMBER,     // lhs.an_member, with `a->b` parsed as `(*a).b`
    AST_NODE_KIND_INIT,       // one flattened initializer: an_val is a byte offset
    AST_NODE_KIND_INITLIST,   // a braced initializer list, its items chained on an_body
    AST_NODE_KIND_DESIGNATOR, // `[an_val]` or `.an_memname` naming where an item lands
    AST_NODE_KIND_ZERO,       // zero an_val bytes of the object an_lhs addresses
```
