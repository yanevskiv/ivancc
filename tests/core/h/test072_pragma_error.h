// A header test072_pragma_error includes twice behind #pragma once.

#pragma once

#ifdef TEST072_ONCE
#read_twice
#endif

#define TEST072_ONCE
