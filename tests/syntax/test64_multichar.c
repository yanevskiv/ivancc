// (Test) Return: 200
// Multi-character constants. Several characters inside one pair of quotes pack
// into an int, the leftmost highest, which the standard leaves to each
// implementation. Only the last four survive, and a wide constant keeps one.

int main()
{
    if ('ab' != 24930) return 1;
    if ('abc' != 6382179) return 2;
    if ('abcd' != 1633837924) return 3;

    // The pack is an int however few characters went into it.
    if (sizeof('ab') != 4) return 4;
    if (sizeof('abcd') != 4) return 5;

    // That int is signed, so a constant filling it comes back negative.
    if ('\xff\xff\xff\xff' != -1) return 6;

    // An escape counts as one character of the pack.
    if ('\n\t' != 2569) return 7;
    if ('\x01\x02' != 258) return 8;
    if ('\101\102' != 16706) return 9;
    if ('\'\'' != 10023) return 10;

    // Only the last four characters reach the int.
    if ('abcde' != 1650680933) return 11;

    // A wide constant has room for one character, and keeps the last.
    if (L'ab' != 98) return 12;
    if (L'\u00e9\u20ac' != 8364) return 13;
    if (sizeof(L'ab') != 4) return 14;

    // A one-character constant is the ordinary case of the same rule.
    if ('a' != 97) return 15;
    if (L'a' != 97) return 16;

    return 200;
}
