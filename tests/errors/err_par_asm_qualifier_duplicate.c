// (Test) Compiler error: [ERR_PAR_ASM_QUALIFIER_DUPLICATE]
// Can't repeat an asm qualifier.

void f(void)
{
#ifdef __x86_64__
    __asm__ volatile inline volatile ("");
#endif
}
