// (Test) Return: 200
// The comma operator: it evaluates both sides and yields the right one, and it
// stays out of the way of the commas that separate arguments.

int add3(int a, int b, int c)
{
    return a + b + c;
}

int main()
{
    int i;
    int j;
    int k;

    i = (1, 2, 3);
    if (i != 3) return 1;

    j = 0;
    k = (j = 5, j + 1);
    if (j != 5) return 2;
    if (k != 6) return 3;

    if (add3(1, 2, 3) != 6) return 4;
    if (add3((1, 10), 2, 3) != 15) return 5;

    for (i = 0, j = 10; i < 3; i++, j--) {
    }
    if (i != 3) return 6;
    if (j != 7) return 7;

    i = 0, j = 0;
    if (i != 0) return 8;

    if (j + k + 17 != 23) return 9;
    return 200;
}
