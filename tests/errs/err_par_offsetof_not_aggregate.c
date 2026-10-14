// (Test) Compiler error: [ERR_PAR_OFFSETOF_NOT_AGGREGATE]
// Can't take the offset of a member of something that is not a struct or union.

unsigned long off = __builtin_offsetof(int, x);
