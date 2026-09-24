// (Test) Return: 200
// The full escape set. A backslash either names a control character, spells a
// byte in octal or hex, or spells a code point as a universal character name.
// A narrow literal holds that code point as its UTF-8 bytes rather than whole.

int main()
{
    char *s;

    if ('\a' != 7) return 1;
    if ('\b' != 8) return 2;
    if ('\f' != 12) return 3;
    if ('\n' != 10) return 4;
    if ('\r' != 13) return 5;
    if ('\t' != 9) return 6;
    if ('\v' != 11) return 7;
    if ('\\' != 92) return 8;
    if ('\'' != 39) return 9;
    if ('\"' != 34) return 10;
    if ('\?' != 63) return 11;

    // An octal escape takes up to three digits and needs no marker.
    if ('\0' != 0) return 12;
    if ('\7' != 7) return 13;
    if ('\10' != 8) return 14;
    if ('\101' != 65) return 15;

    // A plain char is signed, so the top of its range comes back negative.
    if ('\377' != -1) return 16;

    // A hex escape takes as many digits as follow the x.
    if ('\x41' != 65) return 17;
    if ('\x7f' != 127) return 18;
    if ('\xff' != -1) return 19;

    // An escape is one byte of the string, however many characters spell it.
    if (sizeof("\n") != 2) return 20;
    if (sizeof("\101\102\103") != 4) return 21;
    if (sizeof("\x41") != 2) return 22;

    s = "\x41\x42";
    if (s[0] != 65) return 23;
    if (s[1] != 66) return 24;
    if (s[2] != 0) return 25;

    s = "a\tb\nc";
    if (s[1] != 9) return 26;
    if (s[3] != 10) return 27;
    if (sizeof("a\tb\nc") != 6) return 28;

    // A universal character name becomes UTF-8 in a narrow string.
    if (sizeof("\u00e9") != 3) return 29;
    s = "\u00e9";
    if ((unsigned char) s[0] != 195) return 30;
    if ((unsigned char) s[1] != 169) return 31;
    if (s[2] != 0) return 32;

    if (sizeof("\u20ac") != 4) return 33;
    s = "\u20ac";
    if ((unsigned char) s[0] != 226) return 34;
    if ((unsigned char) s[1] != 130) return 35;
    if ((unsigned char) s[2] != 172) return 36;

    if (sizeof("\U0001F600") != 5) return 37;
    s = "\U0001F600";
    if ((unsigned char) s[0] != 240) return 38;
    if ((unsigned char) s[1] != 159) return 39;
    if ((unsigned char) s[2] != 152) return 40;
    if ((unsigned char) s[3] != 128) return 41;

    return 200;
}
