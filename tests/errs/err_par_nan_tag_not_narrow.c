// (Test) Compiler error: [ERR_PAR_NAN_TAG_NOT_NARROW]
// Can't give a NaN builtin a wide string as its tag.

float f = __builtin_nanf(L"1");
