// (Test) Compiler error: [ERR_PAR_ENUM_NOT_CONSTANT]
// An enumerator whose value is a variable.

int n;
enum E { A = n };

int main(void)
{
    return 0;
}
