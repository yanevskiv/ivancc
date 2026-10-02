// (Test) Status: 139
// (Test) Emulator error: [ERR_CPU_WRITE_NOT_MAPPED]
// Can't write to unmapped memory.

int main(void)
{
    volatile int *p = 0;
    *p = 1;
    return 0;
}
