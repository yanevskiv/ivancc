// (Test) Return: 42
// do-while runs its body before testing, so a false condition still runs once.

int main()
{
    int total;
    int i;

    total = 0;
    i = 0;
    do {
        total += i;
        i++;
    } while (i < 5);
    if (total != 10) return 1;

    total = 0;
    do {
        total++;
    } while (0);
    if (total != 1) return 2;

    i = 10;
    do i--; while (i > 6);
    if (i != 6) return 3;

    return i + 36;
}
