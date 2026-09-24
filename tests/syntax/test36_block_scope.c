// (Test) Return: 200
// A nested block shadows the names around it, and each shadowing variable gets
// a frame slot of its own.

int main()
{
    int x;
    int total;

    x = 1;
    total = 0;

    {
        int x;
        x = 10;
        total += x;
        {
            int x;
            x = 100;
            total += x;
        }
        total += x;
    }

    total += x;
    if (total != 121) return 1;

    for (int i = 0; i < 3; i++) {
        int x;
        x = i;
        total += x;
    }
    if (total != 124) return 2;
    if (x != 1) return 3;

    {
        char x;
        x = 'A';
        total = x;
    }

    if (total != 65) return 4;
    return 200;
}
