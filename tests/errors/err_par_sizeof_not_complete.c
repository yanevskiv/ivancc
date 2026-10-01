// (Test) Compiler error: [ERR_PAR_SIZEOF_NOT_COMPLETE]
// Can't take the `sizeof` of an incomplete type.

struct S;
unsigned long n = sizeof(struct S);
