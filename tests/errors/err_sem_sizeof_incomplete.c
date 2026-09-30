// (Test) Compiler error: [ERR_SEM_SIZEOF_INCOMPLETE]
// `sizeof` of an object whose struct is declared but never defined.

struct T;
extern struct T t;

int main(void)
{
    return (int) sizeof t;
}
