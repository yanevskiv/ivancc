// A header core072_pragma_error includes twice behind #pragma once.

#pragma once

#ifdef CORE072_ONCE
#read_twice
#endif

#define CORE072_ONCE
