// (Test) Status: 200
// bug011. A function declared in a block became a local variable of function
// type, so a call through its name jumped to whatever the frame held.

int main()
{
    int g(int);
    int (*p)(int) = g;

    if (g(2) != 20) return 1;
    if (p(3) != 30) return 2;
    return 200;
}

int g(int x)
{
    return x * 10;
}
