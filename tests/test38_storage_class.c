// (Test) Return: 17
// static keeps a name inside this file, and a static local keeps its value
// between calls. register, auto and inline parse and mean nothing.

static int hidden = 7;

static int helper(int n)
{
    return n * 2;
}

int ticker()
{
    static int count = 100;
    count++;
    return count;
}

int main()
{
    register int r = 3;
    auto int q = 4;

    if (hidden != 7) return 1;
    if (helper(4) != 8) return 2;
    if (r + q != 7) return 3;

    if (ticker() != 101) return 4;
    if (ticker() != 102) return 5;
    if (ticker() != 103) return 6;

    return hidden + ticker() - 94;
}
