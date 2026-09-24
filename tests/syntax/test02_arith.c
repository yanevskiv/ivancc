// (Test) Return: 200
// Binary arithmetic and the precedence between it.

int main()
{
    int a = 20;
    int b = 6;

    if (a + b * 2 - 10 / 5 + 17 % 5 != 32) return 1;
    return 200;
}
