// (Test) Return: 200
// Unary minus and logical negation.

int main()
{
    int a = 5;
    int b = -a;
    int c = !0;
    int d = !7;

    if (-b + c * 10 + d != 15) return 1;
    return 200;
}
