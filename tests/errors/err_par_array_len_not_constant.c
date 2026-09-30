// (Test) Compiler error: [ERR_PAR_ARRAY_LEN_NOT_CONSTANT]
// Can't give a file-scope array a length known only at run time.

int n = 3;
int a[n];
