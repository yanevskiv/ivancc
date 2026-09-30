// (Test) Compiler error: [ERR_PAR_SYNTAX]
// bug009. A braced initializer opening with a comma parsed as an empty list,
// and the item after the comma was appended to that NULL list, a crash.

int v[4] = { , 1 };

int main()
{
    return v[0];
}
