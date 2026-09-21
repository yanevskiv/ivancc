// (Test) Return: 44
// File-scope variables: zeroed ones in .bss, initialized ones in .data, both
// addressed off %rip rather than off the frame.

int counter;
int seed = 42;
char letter = 'A';
int table[4];

int bump()
{
    counter += 1;
    return counter;
}

int main()
{
    if (counter != 0) return 1;
    if (seed != 42) return 2;
    if (letter != 'A') return 3;

    bump();
    bump();
    if (counter != 2) return 4;

    seed = seed * 2;
    if (seed != 84) return 5;

    for (int i = 0; i < 4; i++) {
        table[i] = i * i;
    }
    if (table[3] != 9) return 6;

    return counter + seed / 2;
}
