// (Test) Status: 200
// bug000. ivanas assembled `mov (%rax), %eax`, a 32-bit load, as the 64-bit
// `mov (%rax), %rax`, so every unsigned int read through the text path also
// read the 4 bytes after it.

unsigned int pair[2] = { 7, 0xFFFFFFFF };

struct Words {
    unsigned int low;
    unsigned int high;
};

int main()
{
    unsigned int *p = pair;
    unsigned long wide = *p;

    if (wide != 7) return 1;
    if (pair[0] + 1 != 8) return 2;

    struct Words w = { 5, 0xFFFFFFFF };
    unsigned int low = w.low;

    if (low != 5) return 3;
    if ((unsigned long) w.low * 2 != 10) return 4;

    return 200;
}
