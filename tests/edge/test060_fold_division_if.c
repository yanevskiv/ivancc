// (Test) Status: 200
// The preprocessor and the constant folder agree on division by -1: in #if
// and in an enumerator alike, LONG_MIN / -1 wraps to LONG_MIN and
// LONG_MIN % -1 is 0.

#define LONG_MIN (-9223372036854775807L - 1)

#if LONG_MIN / -1 == LONG_MIN && LONG_MIN % -1 == 0
#define PP_AGREES 1
#else
#define PP_AGREES 0
#endif

enum {
    FOLD_AGREES = LONG_MIN / -1 == LONG_MIN && LONG_MIN % -1 == 0
};

int main()
{
    if (PP_AGREES != 1 || FOLD_AGREES != 1) return 1;

    switch (4) {
        case LONG_MIN % -1 + 4: return 200;
    }
    return 2;
}
