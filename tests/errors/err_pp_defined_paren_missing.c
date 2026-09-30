// (Test) Compiler error: [ERR_PP_DEFINED_PAREN_MISSING]
// Can't leave `defined(` without its `)`.

#if defined(X
#endif
