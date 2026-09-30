// (Test) Compiler error: [ERR_PAR_TAG_WRONG_KIND]
// Can't use a struct tag as a union tag.

struct S { int a; };
union S u;
