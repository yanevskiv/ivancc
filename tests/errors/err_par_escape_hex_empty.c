// (Test) Compiler error: [ERR_PAR_ESCAPE_HEX_EMPTY]
// A `\x` escape with no hex digit after it.

char c = '\x';

int main(void)
{
    return 0;
}
