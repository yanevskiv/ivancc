// (Test) Compiler error: [ERR_PAR_OBJECT_LINKAGE]
// An object declared `static` and then without it.

static int x;
int x;

int main(void)
{
    return 0;
}
