// (Test) Return: 200
// A pointer to a pointer gives up one level of indirection at a time.

int main()
{
    int x;
    int *p;
    int **pp;

    x = 5;
    p = &x;
    pp = &p;

    if (*p != 5) return 1;
    if (**pp != 5) return 2;
    if (*pp != p) return 3;

    **pp = 55;
    if (x != 55) return 4;

    if (**pp != 55) return 5;
    return 200;
}
