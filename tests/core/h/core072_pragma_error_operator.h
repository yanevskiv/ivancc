// A header core072_pragma_error includes twice behind a _Pragma("once").

#define CORE072_RUN_ONCE _Pragma("once")

CORE072_RUN_ONCE

#ifdef CORE072_OPERATOR
#read_twice
#endif

#define CORE072_OPERATOR
