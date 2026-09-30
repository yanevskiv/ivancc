// (Test) Compiler warning: [ERR_PAR_ARRAY_ASSUMED_ONE]
// (Test) Status: 0
// A tentative array never given a length takes one element, with a warning.

int a[];

int main(void)
{
    return a[0];
}
