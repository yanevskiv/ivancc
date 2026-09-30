// (Test) Compiler error: [ERR_PAR_INIT_STRING_TOO_LONG]
// Can't initialize an array with a longer string literal.
// Note: gcc only warns, and truncates.

char a[2] = "abc";
