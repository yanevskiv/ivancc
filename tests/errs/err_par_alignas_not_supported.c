// (Test) Compiler error: [ERR_PAR_ALIGNAS_NOT_SUPPORTED]
// Can't align a local above 16.
// Note: gcc aligns the frame for it. Static storage takes up to 4096 here.

void f(void)
{
    _Alignas(32) char aligned;
}
