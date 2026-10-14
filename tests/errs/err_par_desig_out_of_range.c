// (Test) Compiler error: [ERR_PAR_DESIG_OUT_OF_RANGE]
// Can't designate an index past the end of the array.

int a[3] = { [3] = 1 };
