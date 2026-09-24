// (Test) Return: 200
// Several locals coexisting, reassignment, and a chained assignment.

int main()
{
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 0;
    int e = 0;

    d = a + b + c;
    e = d;
    a = b = 7;

    if (a + b + c + d + e != 29) return 1;
    return 200;
}
