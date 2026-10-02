// (Test) Status: 139
// (Test) Emulator error: [ERR_CPU_FETCH_NOT_MAPPED]
// Can't run an instruction from unmapped memory.

int main(void)
{
    int (*volatile f)(void) = 0;
    return f();
}
