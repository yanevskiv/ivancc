// (Test) Status: 200
// Include guards. Including a header again reads it again, so a header gives
// its text every time unless a guard stops it. A guard wraps the whole header
// in #ifndef NAME and #endif, and defines NAME inside. Once NAME is defined, the
// header gives nothing, whatever path names it. Text outside the guard, or an
// #else inside it, still comes through.

#include "h/syntax069_include_guard.h"
#include "h/syntax069_include_guard.h"
#include "h/../h/syntax069_include_guard.h"

int main()
{
    Point p = { 3, 4 };

    if (point_sum(p) != 7) return 1;

    // A header with no guard gives its text every time.
    int plain = 0
#include "h/syntax069_include_guard_plain.h"
#include "h/syntax069_include_guard_plain.h"
#include "h/syntax069_include_guard_plain.h"
    ;
    if (plain != 3) return 2;

    // A guard stops a header until its name is undefined.
    int guarded = 0
#include "h/syntax069_include_guard_count.h"
#include "h/syntax069_include_guard_count.h"
#undef SYNTAX069_INCLUDE_GUARD_COUNT_H
#include "h/syntax069_include_guard_count.h"
#include "h/syntax069_include_guard_count.h"
    ;
    if (guarded != 2) return 3;

    // Text after the guard is outside it.
    int after = 0
#include "h/syntax069_include_guard_after.h"
#include "h/syntax069_include_guard_after.h"
    ;
    if (after != 2) return 4;

    // An #else group is kept once the name is defined.
    int other = 0
#include "h/syntax069_include_guard_else.h"
#include "h/syntax069_include_guard_else.h"
#include "h/syntax069_include_guard_else.h"
    ;
    if (other != 2) return 5;

    return 200;
}
