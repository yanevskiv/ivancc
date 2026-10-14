// (Test) Compiler error: [ERR_SEM_SIZEOF_NOT_COMPLETE]
// Can't take the `sizeof` of an object of incomplete type.

struct T;
extern struct T t;

int main(void)
{
    return (int) sizeof t;
}
