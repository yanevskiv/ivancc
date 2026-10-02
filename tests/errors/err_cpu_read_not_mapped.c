// (Test) Status: 139
// (Test) Emulator error: [ERR_CPU_READ_NOT_MAPPED]
// Can't read unmapped memory.

int main(void)
{
    volatile int *p = 0;
    return *p;
}
