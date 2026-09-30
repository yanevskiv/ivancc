// (Test) Status: 200
// bug002. The constant folder ignored the usual arithmetic conversions and
// never cut a result to its type, so `-1u > 0` folded to 0 and
// `(0u - 1) >> 31` to -1 in enumerators, array lengths, case labels and
// static initializers alike.

enum {
    ALL_ONES      = -1u > 0,
    NOT_ZERO      = ~0u > 0,
    WRAPPED       = 0u - 1 > 0,
    TOP_BIT       = (0u - 1) >> 31,
    LONG_WINS     = -1L > 0u,
    CHAR_PROMOTED = (unsigned char) 200 + 100
};

char lengths[-1u > 0 ? 3 : 5];
unsigned int wrapped = 0u - 1;
long shifted = (0u - 1) >> 31;
long widened = -1L < 0u;

int main()
{
    if (ALL_ONES != 1 || NOT_ZERO != 1 || WRAPPED != 1) return 1;
    if (TOP_BIT != 1 || LONG_WINS != 0 || CHAR_PROMOTED != 300) return 2;
    if (sizeof(lengths) != 3) return 3;
    if (wrapped != 4294967295u || shifted != 1 || widened != 1) return 4;

    switch (1) {
        case (0u - 1) >> 31: return 200;
    }
    return 5;
}
