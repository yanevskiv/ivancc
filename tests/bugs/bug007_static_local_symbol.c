// (Test) Status: 200
// bug007. Block-scope statics of the same name in one function shared one
// symbol and one variable, and a nested one relinked the outer scope's chain.

int main()
{
    int a;
    int b;

    { static int k = 1; k += 10; a = k; }
    { static int k = 2; b = k; }
    if (a != 11 || b != 2) return 1;
    {
        int r = 7;
        static int k = 3;
        {
            int z = 4;
            static int k = 5;
            if (z != 4 || k != 5) return 2;
        }
        if (r != 7 || k != 3) return 3;
    }
    return 200;
}
