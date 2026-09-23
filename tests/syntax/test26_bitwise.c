// (Test) Return: 22
// Bitwise and, or, xor and complement, including their precedence against
// each other: & binds tighter than ^, which binds tighter than |.

int main()
{
    int a;
    int b;

    a = 0xF0;
    b = 0x3C;

    if ((a & b) != 0x30) return 1;
    if ((a | b) != 0xFC) return 2;
    if ((a ^ b) != 0xCC) return 3;
    if ((~a & 0xFF) != 0x0F) return 4;
    if (~0 != -1) return 5;
    if ((3 & 1 | 4 ^ 2) != 7) return 6;
    if (+a != 240) return 7;

    return (a & 0x0F) + (b & 0x0F) + 10;
}
