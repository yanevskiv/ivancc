// (Test) Status: 0
// A test file for <stdio.h> remove and rename.

// Include helpers.
#include "libc18_00_stdio_helpers.inc"

// [C99:7.19.4.2p2] A removed or renamed file should open by its old name no more.
static void Test_StdioFiles(void)
{
    _TEST_TRUE(remove("missing") != 0);
    _TEST_TRUE(rename("missing", "other name") != 0);
    _TEST_TRUE(put("old", "w", "moved"));
    _TEST_INT(rename("old", "new"), 0);
    _TEST_TRUE(fopen("old", "r") == NULL);
    _TEST_TRUE(holds("new", "moved"));
    _TEST_INT(remove("new"), 0);
    _TEST_TRUE(fopen("new", "r") == NULL);
    _TEST_TRUE(remove("new") != 0);
}

int main(void)
{
    Test_StdioFiles();
    return _Test_Status();
}
