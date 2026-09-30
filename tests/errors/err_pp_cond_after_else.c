// (Test) Compiler error: [ERR_PP_COND_AFTER_ELSE]
// Can't have an #elif after #else.

#if 1
#else
#elif 1
#endif
