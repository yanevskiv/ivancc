// (Test) Status: 200
// bug004. A constant sizeof had type int rather than unsigned long, so it
// compared as signed, negated below zero, and was four bytes wide.

int main()
{
    int a[3];

    if (! (sizeof(int) - 5 > 0)) return 1;
    if (sizeof(sizeof(int)) != 8) return 2;
    if (sizeof(sizeof a) != 8) return 3;
    if (-sizeof(a) < 0) return 4;
    if (-sizeof a != 18446744073709551604ul) return 5;
    return 200;
}
