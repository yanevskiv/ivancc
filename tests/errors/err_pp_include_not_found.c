// (Test) Compiler error: [ERR_PP_INCLUDE_NOT_FOUND]
// An #include of a file that is nowhere on the search path.

#include "no_such_header.h"

int main(void)
{
    return 0;
}
