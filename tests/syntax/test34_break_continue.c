// (Test) Return: 200
// break and continue, including that each binds to the innermost loop.

int main()
{
    int total;
    int i;

    total = 0;
    for (i = 0; i < 10; i++) {
        if (i == 3) continue;
        if (i == 6) break;
        total += i;
    }
    if (total != 12) return 1;

    total = 0;
    i = 0;
    while (1) {
        i++;
        if (i > 4) break;
        if (i == 2) continue;
        total += i;
    }
    if (total != 8) return 2;

    total = 0;
    for (int j = 0; j < 3; j++) {
        for (int k = 0; k < 3; k++) {
            if (k == 1) break;
            total += 1;
        }
        total += 10;
    }
    if (total != 33) return 3;

    i = 0;
    do {
        i++;
        if (i < 3) continue;
        break;
    } while (1);
    if (i != 3) return 4;

    if (total + 9 != 42) return 5;
    return 200;
}
