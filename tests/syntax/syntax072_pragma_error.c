// (Test) Compiler error: [ERR_PP_ERROR_DIRECTIVE]
// Pragmas and diagnostics. A header that runs #pragma once is never read again.
// _Pragma runs a string as a #pragma, even from inside a macro. Every other
// pragma is dropped. #warning prints its message and carries on, and #error
// prints its message and fails the compile.

#include "h/syntax072_pragma_error.h"
#include "h/syntax072_pragma_error.h"
#include "h/syntax072_pragma_error_operator.h"
#include "h/syntax072_pragma_error_operator.h"

#define PRAGMA(x) _Pragma(#x)

#pragma STDC FP_CONTRACT ON
#pragma pack(1)
#pragma
_Pragma("GCC diagnostic push")
PRAGMA(pack(push, 1))

#warning the pragmas were run
#error the compile stops here
