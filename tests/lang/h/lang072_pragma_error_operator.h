// A header lang072_pragma_error includes twice behind a _Pragma("once").

#define LANG072_RUN_ONCE _Pragma("once")

LANG072_RUN_ONCE

#ifdef LANG072_OPERATOR
#read_twice
#endif

#define LANG072_OPERATOR
