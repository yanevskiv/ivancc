// (Test) Compiler error: [ERR_PAR_SIZEOF_INCOMPLETE]
// Can't take the `sizeof` of an incomplete type.

struct S;
unsigned long n = sizeof(struct S);
