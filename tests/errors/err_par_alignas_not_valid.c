// (Test) Compiler error: [ERR_PAR_ALIGNAS_NOT_VALID]
// Can't give `_Alignas` anything but zero or a power of two up to 4096.

_Alignas(3) char aligned;
