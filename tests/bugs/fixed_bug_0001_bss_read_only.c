// (Test) Status: 200
// bug_0001. ivas made an unflagged `.section .bss` a read-only PROGBITS
// section, where GNU as makes it writable NOBITS. ivcc -S writes .bss that
// way, so writing a zero-initialized global crashed on the text path.

int counter;
static long total;
char buffer[64];

int bump(void)
{
    static int calls;

    return ++calls;
}

int main()
{
    counter = 5;
    total = counter * 2;
    buffer[63] = 'z';

    if (counter != 5) return 1;
    if (total != 10) return 2;
    if (buffer[63] != 'z' || buffer[0] != 0) return 3;

    bump();
    if (bump() != 2) return 4;

    return 200;
}
