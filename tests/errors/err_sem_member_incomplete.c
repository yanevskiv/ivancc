// (Test) Compiler error: [ERR_SEM_MEMBER_INCOMPLETE]
// Can't access a member of an incomplete type.

struct T;
extern struct T t;

int main(void)
{
    return t.a;
}
