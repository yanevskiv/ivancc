// (Test) Return: 46
// Prefix and postfix ++ and --: which value each yields, and that ++ on a
// pointer steps by the size of what it points at.

int main()
{
    int i;
    char *p;
    int arr[3];
    int total;

    i = 5;
    if (i++ != 5) return 1;
    if (i != 6) return 2;
    if (++i != 7) return 3;
    if (i-- != 7) return 4;
    if (i != 6) return 5;
    if (--i != 5) return 6;

    p = "abc";
    if (*p++ != 'a') return 7;
    if (*p != 'b') return 8;
    ++p;
    if (*p != 'c') return 9;

    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    total = 0;
    for (i = 0; i < 3; ++i) {
        total += arr[i];
    }
    if (total != 60) return 10;

    i = 0;
    total = arr[i++];
    if (i != 1) return 11;
    if (total != 10) return 12;

    return total + i * 36;
}
