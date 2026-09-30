// (Test) Compiler error: [ERR_PAR_ENUM_NOT_CONSTANT]
// Can't give an enumerator a value known only at run time.

int n;
enum E { A = n };
