// (Test) Compiler error: [ERR_SEM_SIZEOF_INCOMPLETE]
// Can't take the `sizeof` of an object of incomplete type.

struct T;
extern struct T t;

int main(void)
{
    return (int) sizeof t;
}
