// (Test) Compiler error: [ERR_SEM_ASSIGN_CONST]
// bug022. A qualifier written after a pointer's star was parsed and dropped, so
// a const pointer could be reassigned.

int x;
int y;

int main()
{
    int * const p = &x;

    p = &y;
    return 0;
}
