// (Test) Return: 200
// Wide literals. An L prefix makes every element a wchar_t, which is an int
// here, so a wide string is an int array and a wide character constant holds
// the code point itself. A narrow piece spliced onto a wide one widens to it.

int main()
{
    int *w;

    if (L'A' != 65) return 1;
    if (sizeof(L'A') != 4) return 2;
    if (L'\n' != 10) return 3;
    if (L'\101' != 65) return 4;

    // A wide constant has room for the byte a narrow one turns negative.
    if (L'\xff' != 255) return 5;
    if (L'\377' != 255) return 6;

    // A universal character name stays one element rather than becoming UTF-8.
    if (L'\u00e9' != 233) return 7;
    if (L'\u20ac' != 8364) return 8;

    // Four bytes an element, and one more element for the terminator.
    if (sizeof(L"abc") != 16) return 9;
    if (sizeof(L"") != 4) return 10;

    w = L"abc";
    if (w[0] != 97) return 11;
    if (w[1] != 98) return 12;
    if (w[2] != 99) return 13;
    if (w[3] != 0) return 14;

    w = L"\u20ac";
    if (sizeof(L"\u20ac") != 8) return 15;
    if (w[0] != 8364) return 16;
    if (w[1] != 0) return 17;

    w = L"\U0001F600";
    if (w[0] != 128512) return 18;

    // An escape occupies one element of a wide string, as it does one byte.
    w = L"a\tb";
    if (w[1] != 9) return 19;
    if (sizeof(L"a\tb") != 16) return 20;

    // A splice is wide when either piece is, and the narrow side widens.
    if (sizeof(L"ab" "cd") != 20) return 21;
    w = L"ab" "cd";
    if (w[2] != 99) return 22;
    if (w[4] != 0) return 23;

    if (sizeof("ab" L"cd") != 20) return 24;
    w = "ab" L"cd";
    if (w[0] != 97) return 25;
    if (w[3] != 100) return 26;

    // Two wide pieces join the way two narrow ones do.
    if (sizeof(L"ab" L"cd") != 20) return 27;

    // The element type is what makes subscripting step four bytes at a time.
    if (sizeof(L"abc"[0]) != 4) return 28;

    return 200;
}
