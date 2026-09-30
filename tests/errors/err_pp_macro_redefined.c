// (Test) Compiler warning: [ERR_PP_MACRO_REDEFINED]
// (Test) Status: 2
// A macro defined again with a different body warns, and the new body wins.

#define VALUE 1
#define VALUE 2

int main(void)
{
    return VALUE;
}
