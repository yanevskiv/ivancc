// (Test) Compiler error: [ERR_PAR_INIT_EMPTY]
// Can't initialize an `int` with empty braces.
// Note: gcc accepts it, as C23 does.

int x = { };
