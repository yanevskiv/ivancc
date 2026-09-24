// (Test) Return: 200
// Compound statements nest, and an empty one is legal.

int main()
{
    int r = 1;

    {
        r = r + 1;
        {
            r = r * 10;
        }
    }

    { }

    if (1) {
        r = r + 5;
    }

    if (r != 25) return 1;
    return 200;
}
