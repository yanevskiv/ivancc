// (Test) Compiler error: [ERR_PAR_ARRAY_DECOR_NOT_OUTERMOST]
// Can't put `static` on any array of a parameter but the outermost.

void f(int a[3][static 3]);
