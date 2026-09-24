// (Test) Return: 200
// Calls with arguments, nested calls, and a call used as an argument.

int add(int a, int b)
{
    return a + b;
}

int square(int x)
{
    return x * x;
}

int main()
{
    if (add(square(3), add(2, 5)) != 16) return 1;
    return 200;
}
