// (Test) Compiler error: [ERR_PAR_ENUM_REDECLARED]
// bug021. An enumerator declared twice in one scope compiled, and the later
// value won.

enum Color { RED, GREEN, RED };

int main()
{
    return RED;
}
