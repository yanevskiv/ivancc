// (Test) Compiler warning: [ERR_PP_MACRO_REDEFINED]
// (Test) Status: 2
// Shouldn't redefine a macro with a different body. The new body wins.

#define VALUE 1
#define VALUE 2

int main(void)
{
    return VALUE;
}
