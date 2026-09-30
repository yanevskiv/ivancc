// (Test) Compiler error: [ERR_PP_INCLUDE_TOO_DEEP]
// A file that includes itself with no guard nests past the limit.

#include "err_pp_include_too_deep.c"

int main(void)
{
    return 0;
}
