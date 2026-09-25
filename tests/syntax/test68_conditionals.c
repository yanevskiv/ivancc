// (Test) Return: 200
// Conditionals. #if keeps its group when its expression is not zero, and each
// #elif and the #else after it offer the next group in turn. #ifdef, #ifndef
// and the defined operator ask whether a name is a macro. The expression is
// computed in the widest integer types, and a name that is not a macro counts
// as zero. A skipped group may hold any text, and its directives are not run.

#define TEN 10
#define EMPTY
#define ZERO 0
#define ADD(a, b) ((a) + (b))

int main()
{
    int kept = 0;

#if 1
    kept = 1;
#else
    return 1;
#endif
    if (kept != 1) return 2;

    // Only the first group whose expression is not zero is kept.
#if 0
    return 3;
#elif 0
    return 4;
#elif 2
    kept = 2;
#elif 1
    return 5;
#else
    return 6;
#endif
    if (kept != 2) return 7;

    // A macro defined as nothing is still defined.
#ifdef TEN
    kept = 3;
#endif
#ifndef TEN
    return 8;
#endif
#ifdef ELEVEN
    return 9;
#endif
#ifndef EMPTY
    return 10;
#endif
    if (kept != 3) return 11;

#if ! defined TEN || ! defined(ADD) || defined ELEVEN || defined(ELEVEN)
    return 12;
#endif

#undef ZERO
#ifdef ZERO
    return 13;
#endif

    // The conditionals inside a skipped group are skipped whole.
#if 0
#if 1
    return 14;
#else
    return 15;
#endif
#elif 1
#if 0
    return 16;
#endif
    kept = 4;
#endif
    if (kept != 4) return 17;

    // A skipped group is never lexed as C, and its directives never run.
#if 0
    don't "stop
#include "inc/no_such_file.h"
#frobnicate
#define TEN 11
#endif
    if (TEN != 10) return 18;

    // An #elif after a kept group is never evaluated.
#if 1
#elif 1 / 0
    return 19;
#endif

    // Macros expand, and every name left over counts as zero.
#if ADD(TEN, 2) != 12 || TEN * 2 != 20
    return 20;
#endif
#if UNDEFINED || UNDEFINED != 0
    return 21;
#endif

#if 1 + 2 * 3 != 7 || (1 + 2) * 3 != 9 || 10 - 4 - 3 != 3 || 100 / 10 / 5 != 2
    return 22;
#endif
#if 1 << 4 != 16 || 256 >> 4 != 16 || 17 % 5 != 2
    return 23;
#endif
#if (6 & 3) != 2 || (6 | 3) != 7 || (6 ^ 3) != 5 || ~0 != -1
    return 24;
#endif
#if ! (1 < 2) || 2 <= 1 || ! (2 >= 2) || 1 > 2 || -3 != 0 - 3 || +3 != 3
    return 25;
#endif
#if (1 ? 2 : 3) != 2 || (0 ? 2 : 3) != 3 || (0 ? 1 : 0 ? 2 : 3) != 3
    return 26;
#endif

    // Signed values are intmax_t, and unsigned ones are uintmax_t.
#if 2147483647 + 1 < 0 || 1 << 62 < 0 || -9223372036854775807 - 1 >= 0
    return 27;
#endif
#if -1 < 0u || 0xFFFFFFFFFFFFFFFF != -1 || 18446744073709551615u / 2 != 9223372036854775807
    return 28;
#endif
#if (-7) / 2 != -3 || (-7) % 2 != -1 || -16 >> 2 != -4
    return 29;
#endif
#if 10u != 10 || 10UL != 10 || 10ll != 10 || 10LLU != 10 || 0x10 != 16 || 010 != 8
    return 30;
#endif

    // The operand that is not evaluated may divide by zero.
#if 0 && 1 / 0
    return 31;
#endif
#if ! (1 || 1 / 0)
    return 32;
#endif
#if (1 ? 2 : 1 / 0) != 2 || (0 ? 1 % 0 : 3) != 3
    return 33;
#endif

#if 'A' != 65 || '\n' != 10 || '\0' != 0 || '\377' != -1 || L'\377' != 255
    return 34;
#endif

    return 200;
}
