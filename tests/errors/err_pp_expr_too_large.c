// (Test) Compiler error: [ERR_PP_EXPR_TOO_LARGE]
// Can't use an integer in #if too large for any type.
// Note: gcc only warns.

#if 18446744073709551616
#endif
