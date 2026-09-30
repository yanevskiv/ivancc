// (Test) Compiler error: [ERR_PAR_ESCAPE_UCN_INCOMPLETE]
// A `\u` escape with two hex digits rather than four.

char *s = "\u12";

int main(void)
{
    return 0;
}
