// (Test) Compiler error: [ERR_SEM_ASSIGN_CONST]
// A struct holding a const member cannot be assigned as a whole.

struct S { const int id; int value; };

int main()
{
    struct S a = { 1, 2 };
    struct S b = { 3, 4 };

    a = b;
    return a.value;
}
