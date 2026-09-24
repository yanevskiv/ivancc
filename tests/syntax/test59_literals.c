// (Test) Return: 200
// Integer literals. A leading zero makes a constant octal, and a suffix fixes
// its signedness and its least width. Where no suffix says otherwise the value
// itself picks the first type of the list that can hold it.

int main()
{
    if (0 != 0) return 1;
    if (00 != 0) return 2;
    if (07 != 7) return 3;
    if (010 != 8) return 4;
    if (0777 != 511) return 5;
    if (01000 != 512) return 6;

    // Octal, hex and decimal are three spellings of one value.
    if (0x1F != 31) return 7;
    if (037 != 31) return 8;
    if (31 != 0x1f) return 9;

    // A leading zero binds tighter than the digits after it suggest.
    if (0123 != 83) return 10;

    if (10u != 10) return 11;
    if (10U != 10) return 12;
    if (10l != 10) return 13;
    if (10L != 10) return 14;
    if (10ll != 10) return 15;
    if (10LL != 10) return 16;
    if (10ul != 10) return 17;
    if (10UL != 10) return 18;
    if (10ull != 10) return 19;
    if (10llu != 10) return 20;

    // A suffix is what makes the literal's own type unsigned.
    if (sizeof(1) != 4) return 21;
    if (sizeof(1u) != 4) return 22;
    if (sizeof(1l) != 8) return 23;
    if (sizeof(1ul) != 8) return 24;
    if (sizeof(1ll) != 8) return 25;
    if (sizeof(0x7FFFFFFF) != 4) return 26;

    // A decimal too wide for an int lands on a long instead.
    if (sizeof(4294967296) != 8) return 27;
    if (4294967296 / 65536 != 65536) return 28;

    // The unsignedness of a literal reaches the operator it feeds.
    if (-1 < 1u) return 29;
    if (! (-1 > 1u)) return 30;
    if (-1 / 2u != 2147483647) return 31;

    if (0xFFFFFFFFu >> 16 != 65535) return 32;
    if (0xFFFFFFFFFFFFFFFFul / 3 != 6148914691236517205) return 33;

    // A character constant is an int holding the byte it names.
    if ('A' != 65) return 34;
    if (sizeof('A') != 4) return 35;
    if ('\n' != 10) return 36;

    long l = 01777777777777777777777ul;
    if (l != -1) return 37;

    int arr[010];
    if (sizeof(arr) != 32) return 38;

    switch (012) {
        case 010: {
            return 39;
        } break;
        case 012: {
            // empty
        } break;
        default: {
            return 40;
        }
    }

    unsigned int u = 0xDEADBEEF;
    if (u != 3735928559) return 41;
    if ((u >> 28) != 13) return 42;

    return 0310;
}
