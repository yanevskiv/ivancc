// (Test) Compiler error: [ERR_PAR_ALIGNAS_NOT_ALLOWED]
// Can't align a typedef.

typedef _Alignas(8) char Aligned;
