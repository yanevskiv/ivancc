// (Test) Return: 200
// Adjacent string literals. Two literals written next to each other are one
// literal, spliced before anything reads a type or counts a length. Each side
// decodes its own escapes first, so a numeric escape cannot run into the next.

int main()
{
    char *s;

    if (sizeof("ab" "cd") != 5) return 1;
    s = "ab" "cd";
    if (s[0] != 'a') return 2;
    if (s[2] != 'c') return 3;
    if (s[3] != 'd') return 4;
    if (s[4] != 0) return 5;

    // Three pieces join as readily as two.
    if (sizeof("a" "b" "c") != 4) return 6;
    s = "a" "b" "c";
    if (s[1] != 'b') return 7;

    // An empty piece contributes nothing but its own absence of bytes.
    if (sizeof("ab" "") != 3) return 8;
    if (sizeof("" "ab") != 3) return 9;
    if (sizeof("" "") != 1) return 10;

    // A hex escape stops at its own literal's closing quote.
    s = "\x4" "1";
    if (s[0] != 4) return 11;
    if (s[1] != '1') return 12;
    if (sizeof("\x4" "1") != 3) return 13;

    // Without the split those two pieces would read as one escape.
    if (sizeof("\x41") != 2) return 14;

    // An octal escape stops there too.
    s = "\1" "23";
    if (s[0] != 1) return 15;
    if (s[1] != '2') return 16;

    // A splice holds its embedded escapes the way one literal would.
    s = "a\n" "\tb";
    if (s[1] != 10) return 17;
    if (s[2] != 9) return 18;
    if (sizeof("a\n" "\tb") != 5) return 19;

    // The result is an array, not a pointer, so sizeof still counts bytes.
    if (sizeof("hello" " " "world") != 12) return 20;

    return 200;
}
