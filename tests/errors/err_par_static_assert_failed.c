// (Test) Compiler error: [ERR_PAR_STATIC_ASSERT_FAILED]
// Can't compile past a static assertion that does not hold.

_Static_assert(sizeof(int) == 8, "an int is eight bytes");
