// A header test72_pragma_error includes twice behind a _Pragma("once").

#define TEST72_RUN_ONCE _Pragma("once")

TEST72_RUN_ONCE

#ifdef TEST72_OPERATOR
#read_twice
#endif

#define TEST72_OPERATOR
