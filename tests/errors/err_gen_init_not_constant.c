// (Test) Compiler error: [ERR_GEN_INIT_NOT_CONSTANT]
// Can't initialize a file-scope object with a value known only at run time.

int x;
int y = x;
