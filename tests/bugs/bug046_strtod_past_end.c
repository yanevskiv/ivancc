// (Test) Status: 0
// bug046. strtod read the character after the one it stood on before it knew
// the string went on, looking for the `x` of a hexadecimal number, so an
// empty string and a lone sign were read one byte past the terminator. A
// string that ends where the break's last page ends killed the program with
// SIGSEGV. Each string here ends there when strtod reads it.

#include <stdlib.h>
#include <_sys.h>

#define PAGE 4096

// A copy of str that ends on the last byte of a new last page of the break.
static char *at_end(const char *str, unsigned long len)
{
    unsigned long base = (unsigned long) _Sys_InvokeBrk(0);
    unsigned long end = (base + PAGE + PAGE - 1) / PAGE * PAGE;
    char *tail = (char *) end - len - 1;

    if ((unsigned long) _Sys_InvokeBrk((void *) end) != end) {
        return 0;
    }
    for (unsigned long i = 0; i <= len; i++) {
        tail[i] = str[i];
    }
    return tail;
}

// strtod should take used characters of str and read none past its end.
static int takes(const char *str, unsigned long len, unsigned long used)
{
    char *tail = at_end(str, len);
    char *stop = 0;

    return tail && strtod(tail, &stop) == 0.0 && stop == tail + used;
}

int main(void)
{
    if (! takes("", 0, 0)) {
        return 1;
    }
    if (! takes("-", 1, 0)) {
        return 2;
    }
    if (! takes(" +", 2, 0)) {
        return 3;
    }
    if (! takes("0", 1, 1)) {
        return 4;
    }
    if (! takes("0x", 2, 1)) {
        return 5;
    }
    if (! takes("0x.", 3, 1)) {
        return 6;
    }
    if (! takes("0e", 2, 1)) {
        return 7;
    }
    if (! takes("0e+", 3, 1)) {
        return 8;
    }
    if (! takes(".", 1, 0)) {
        return 9;
    }
    if (! takes("i", 1, 0)) {
        return 10;
    }
    if (! takes("n", 1, 0)) {
        return 11;
    }
    return 0;
}
