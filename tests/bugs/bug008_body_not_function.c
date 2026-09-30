// (Test) Compiler error: [ERR_PAR_BODY_NOT_FUNCTION]
// bug008. A body after a declarator that is not a function crashed the
// compiler, which built a function with no name from stale state.

int main{ return 0; }
