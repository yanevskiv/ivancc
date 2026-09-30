// (Test) Compiler error: [ERR_PP_INCLUDE_TOO_DEEP]
// Can't nest #include past the limit, as a file including itself does.

#include "err_pp_include_too_deep.c"
