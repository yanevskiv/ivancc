// A header syntax072_pragma_error includes twice behind a _Pragma("once").

#define SYNTAX072_RUN_ONCE _Pragma("once")

SYNTAX072_RUN_ONCE

#ifdef SYNTAX072_OPERATOR
#read_twice
#endif

#define SYNTAX072_OPERATOR
