// (Test) Return: 31
// The conditional operator, including nesting and its precedence against
// assignment: a = b ? c : d assigns the whole conditional.

int max(int a, int b)
{
    return a > b ? a : b;
}

int main()
{
    int a;
    int b;
    char *s;

    a = 3;
    b = 9;

    if ((a > b ? 1 : 0) != 0) return 1;
    if ((a < b ? 1 : 0) != 1) return 2;
    if (max(a, b) != 9) return 3;
    if (max(b, a) != 9) return 4;
    if ((1 ? 2 : 3) + (0 ? 4 : 5) != 7) return 5;
    if ((a ? (b ? 10 : 20) : 30) != 10) return 6;

    b = a > 2 ? 100 : 200;
    if (b != 100) return 7;

    s = a == 3 ? "yes" : "no";
    if (*s != 'y') return 8;

    return max(a, b) - 69;
}
