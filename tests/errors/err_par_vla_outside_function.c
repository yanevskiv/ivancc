// (Test) Compiler error: [ERR_PAR_VLA_OUTSIDE_FUNCTION]
// Can't size a variable-length array outside a function.

int n;
int s = sizeof(int[n]);
