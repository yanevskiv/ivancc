// (Test) Return: 200
// Several declarators in one declaration, at file scope and inside a function,
// with and without initializers.

int a, b = 2, c;
char p = 'x', q;

int main()
{
    int x, y = 5, z;
    char m = 'A', n = 'B';

    if (a != 0 || c != 0) return 1;
    if (b != 2) return 2;
    if (p != 'x' || q != 0) return 3;

    x = 1;
    z = x + y;
    if (z != 6) return 4;
    if (m != 'A' || n != 'B') return 5;

    for (int i = 0, j = 3; i < j; i++) {
        z += i;
    }
    if (z != 9) return 6;

    if (z + b + m - 37 != 39) return 7;
    return 200;
}
