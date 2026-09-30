// (Test) Compiler error: [ERR_PP_EXPR_COLON_MISSING]
// Can't have a `?` without its `:` in #if.

#if 1 ? 2
#endif
