// (Test) Return: 44
// File-scope variables: zeroed ones in .bss, initialized ones in .data, both
// addressed off %rip rather than off the frame. An initializer here is folded
// whole, so any constant expression serves and not only a literal, and an
// address among them becomes a relocation the linker fills in.

int counter;
int seed = 42;
char letter = 'A';
int table[4];
int below = -19;
int folded = 2 * 20 + 2;
int chosen = 1 ? 7 : 9;
int sized = sizeof(int) * 2;
char narrowed = (char) 300;
char *greeting = "hi";
int *pointed = &seed;
int *element = table;

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
    if (below != -19) return 8;
    if (folded != 42) return 9;
    if (chosen != 7) return 10;
    if (sized != 8) return 11;
    if (narrowed != 44) return 12;
    if (greeting[0] != 'h' || greeting[1] != 'i' || greeting[2] != 0) return 13;
    if (*pointed != 42) return 14;
    if (element != table) return 15;

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
