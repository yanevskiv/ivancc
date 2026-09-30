// (Test) Status: 200
// bug013. A type name had no function declarator, so casts to function pointer
// types, as `<signal.h>` spells SIG_DFL, and their sizes were syntax errors.

int twice(int x) { return 2 * x; }

int main()
{
    int (*f)(int) = (int (*)(int)) twice;
    void (*dfl)(int) = (void (*)(int)) 0;
    int (*(*table)[3])(void) = 0;

    if (f(4) != 8) return 1;
    if (dfl) return 2;
    if (sizeof(int (*[3])(void)) != 24) return 3;
    if (sizeof(int (*(*)(void))[3]) != 8) return 4;
    if (sizeof(int ((*))) != 8) return 5;
    if (table != (int (*(*)[3])(void)) 0) return 6;
    return 200;
}
