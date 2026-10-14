// A header lang072_pragma_error includes twice behind #pragma once.

#pragma once

#ifdef LANG072_ONCE
#read_twice
#endif

#define LANG072_ONCE
