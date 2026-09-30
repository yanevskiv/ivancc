// (Test) Compiler error: [ERR_PAR_INIT_TOO_MANY_ELEMENTS]
// Can't give an array more initializers than elements.
// Note: gcc only warns, and drops the rest.

int a[2] = { 1, 2, 3 };
