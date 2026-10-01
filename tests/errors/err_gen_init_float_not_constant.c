// (Test) Compiler error: [ERR_GEN_INIT_FLOAT_NOT_CONSTANT]
// Can't initialize a file-scope `double` with a value known only at run time.

double x;
double y = x;
