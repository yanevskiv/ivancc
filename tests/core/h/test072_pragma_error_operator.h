// A header test072_pragma_error includes twice behind a _Pragma("once").

#define TEST072_RUN_ONCE _Pragma("once")

TEST072_RUN_ONCE

#ifdef TEST072_OPERATOR
#read_twice
#endif

#define TEST072_OPERATOR
