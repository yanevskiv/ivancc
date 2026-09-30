// (Test) Compiler error: [ERR_PP_EXPR_VALUE_MISSING]
// Can't leave a binary operator in #if without its right operand.

#if 1 +
#endif
