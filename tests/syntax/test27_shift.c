// (Test) Return: 200
// Shifts, including the arithmetic right shift a signed operand requires and
// the count travelling through %cl.

int shift_right(int x, int n)
{
    return x >> n;
}

int main()
{
    int n;

    n = 3;

    if ((1 << 5) != 32) return 1;
    if ((1 << n) != 8) return 2;
    if ((240 >> 4) != 15) return 3;
    if (shift_right(-16, 2) != -4) return 4;
    if (shift_right(-1, 20) != -1) return 5;
    if ((2 << 3 >> 1) != 8) return 6;
    if ((1 << 2 + 1) != 8) return 7;

    if ((1 << n) + shift_right(248, 3) != 39) return 8;
    return 200;
}
