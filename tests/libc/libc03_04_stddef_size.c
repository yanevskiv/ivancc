// (Test) Status: 0
// A test file for <stddef.h> size_t type.

// Include helpers.
#include "libc03_00_stddef_helpers.inc"

// (S7.17p2) size_t should be unsigned and hold what sizeof yields.
static void Test_StddefSize(void)
{
    size_t size = sizeof(struct Pair);

    _TEST_TRUE((size_t) -1 > 0);
    _TEST_UINT(sizeof(size_t), sizeof(sizeof(int)));
    _TEST_UINT(size, sizeof(struct Pair));
}

int main(void)
{
    Test_StddefSize();
    return _Test_Status();
}
