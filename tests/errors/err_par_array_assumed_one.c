// (Test) Compiler warning: [ERR_PAR_ARRAY_ASSUMED_ONE]
// (Test) Status: 0
// Shouldn't leave a tentative array without a length. It gets one element.

int a[];

int main(void)
{
    return a[0];
}
