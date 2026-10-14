// (Test) Compiler error: [ERR_PAR_DESIG_NOT_AGGREGATE]
// Can't designate a member in an array's initializer.

int a[2] = { .x = 1 };
