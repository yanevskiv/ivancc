// (Test) Compiler error: [ERR_PAR_ALIGNAS_NOT_CONSTANT]
// Can't give `_Alignas` anything but an integer constant expression.

int n = 8;
_Alignas(n) char aligned;
