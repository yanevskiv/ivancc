// (Test) Compiler error: [ERR_PAR_ARRAY_DECOR_NOT_PARAM]
// `static` inside the brackets of an array that is no parameter.

int a[static 3];

int main(void)
{
    return 0;
}
