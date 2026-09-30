// (Test) Compiler error: [ERR_PAR_SPEC_TWO_TYPES]
// A typedef name and a type keyword in one declaration.

typedef int T;
T double y;

int main(void)
{
    return 0;
}
