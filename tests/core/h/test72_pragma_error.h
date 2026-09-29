// A header test72_pragma_error includes twice behind #pragma once.

#pragma once

#ifdef TEST72_ONCE
#read_twice
#endif

#define TEST72_ONCE
