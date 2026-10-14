// (Test) Compiler error: [ERR_SEM_DIVISION_BY_ZERO]
// Can't divide by zero in a constant expression.

enum { A = 1 / 0 };
