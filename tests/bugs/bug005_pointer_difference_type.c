// (Test) Status: 200
// bug005. Pointer subtraction had type int rather than long, so the difference
// was four bytes wide and lost its high bits.

int main()
{
    int a[4];
    int *p = a + 3;
    int *q = a;
    char *lo = (char *) 0;
    char *hi = (char *) 0x180000000L;

    if (sizeof(p - q) != 8) return 1;
    if (q - p != -3) return 2;
    if (hi - lo != 0x180000000L) return 3;
    if ((hi - lo) / 2 != 0xc0000000L) return 4;
    return 200;
}
