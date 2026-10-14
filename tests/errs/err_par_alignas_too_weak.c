// (Test) Compiler error: [ERR_PAR_ALIGNAS_TOO_WEAK]
// Can't align an object below what its type requires.

_Alignas(2) int aligned;
