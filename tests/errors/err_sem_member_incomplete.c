// (Test) Compiler error: [ERR_SEM_MEMBER_INCOMPLETE]
// A member of an object whose struct is declared but never defined.

struct T;
extern struct T t;

int main(void)
{
    return t.a;
}
