// (Test) Compiler error: [ERR_PAR_NAN_TAG_NOT_EMPTY]
// Can't give a NaN builtin a payload yet.

float f = __builtin_nanf("1");
