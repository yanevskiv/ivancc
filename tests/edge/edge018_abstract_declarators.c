// (Test) Status: 200
// Type names with function declarators, nested parentheses and unsized arrays,
// in casts, sizeof, va_arg and compound literals.

typedef __builtin_va_list va_list;

int inc(int x) { return x + 1; }
int dec(int x) { return x - 1; }
int (*pick(void))(int) { return dec; }
int compare(const void *a, const void *b) { return *(const int *) a - *(const int *) b; }

int apply(int n, ...)
{
    va_list ap;
    int (*f)(int);

    __builtin_va_start(ap, n);
    f = __builtin_va_arg(ap, int (*)(int));
    __builtin_va_end(ap);
    return f(n);
}

int main()
{
    int a = 3;
    int b = 5;
    void *raw = (void *) inc;
    int (*f)(int) = (int (*)(int x)) raw;
    int (*(*g)(void))(int) = (int (*(*)(void))(int)) pick;
    int (*cmp)(const void *, const void *) = (int (*)(const void *, const void *)) compare;
    int (**table)(int) = (int (*[2])(int)) { inc, dec };
    int *list = (int []) { 7, 8, 9 };
    char *word = (char []) { "hey" };

    if (f(1) != 2 || g()(1) != 0) return 1;
    if (cmp(&a, &b) >= 0) return 2;
    if (table[0](10) != 11 || table[1](10) != 9) return 3;
    if (list[2] != 9 || word[1] != 'e') return 4;
    if (sizeof((int []) { 1, 2, 3, 4 }) != 16) return 5;
    if (sizeof(int (*[3])(void)) != 24 || sizeof(int (*)[]) != 8) return 6;
    if (sizeof(char ((((*))))) != 8 || sizeof(char (([2]))) != 2) return 7;
    if (sizeof(int (*(*)(void))[3]) != 8 || sizeof(long [2][3]) != 48) return 8;
    if (apply(41, inc) != 42) return 9;
    if ((int (*)(int, ...)) 0 != 0) return 10;
    return 200;
}
