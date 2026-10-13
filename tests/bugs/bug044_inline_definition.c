// (Test) Status: 0
// bug044. `inline` was ignored, so an inline definition became an external
// one, and two translation units that included it failed to link. A function
// whose file-scope declarations all say `inline` and none `extern` has an
// inline definition, which gives no external one [C99 S6.7.4p7]: `twice` below
// links with the external definition the asm gives. A declaration without
// `inline`, or with `extern`, makes the definition external again.

inline long twice(long n)
{
    return n + n;
}

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl twice\n"
         "twice: leaq (%rdi), %rax; addq %rdi, %rax\n"
         "ret");
#endif

inline long thrice(long n)
{
    return n + n + n;
}

extern long thrice(long n);

extern inline long fourfold(long n)
{
    return 4 * n;
}

long fivefold(long n);

inline long fivefold(long n)
{
    return 5 * n;
}

inline long sixfold(long n);

long sixfold(long n)
{
    return 6 * n;
}

static inline long sevenfold(long n)
{
    return 7 * n;
}

int main(void)
{
    long (*taken)(long) = twice;
    long sevenfold(long n);
    long sum = 0;

    if (twice(21) != 42 || taken(4) != 8) {
        return 1;
    }
    if (thrice(2) != 6 || fourfold(2) != 8 || fivefold(2) != 10) {
        return 2;
    }
    if (sixfold(2) != 12 || sevenfold(2) != 14) {
        return 3;
    }
    taken = thrice;
    sum += taken(1);
    taken = fourfold;
    sum += taken(1);
    taken = sevenfold;
    sum += taken(1);
    return sum == 14 ? 0 : 4;
}
