// (Test) Return: 200
// char is one byte and signed, and packs beside an int without disturbing it.

int main()
{
    char a;
    char b;
    int  n;

    a = 300;
    b = 200;
    n = 1000;

    if (b >= 0) return 1;
    if (n != 1000) return 2;

    if (a + 49 != 93) return 3;
    return 200;
}
