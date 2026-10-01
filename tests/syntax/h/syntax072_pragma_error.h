// A header syntax072_pragma_error includes twice behind #pragma once.

#pragma once

#ifdef SYNTAX072_ONCE
#read_twice
#endif

#define SYNTAX072_ONCE
