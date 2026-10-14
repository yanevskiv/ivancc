// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_X87_DB_NOT_IMPLEMENTED]
// Can't run a DB x87 operation the CPU does not implement.
// Note: DB F8 is reserved, so the hardware raises #UD too.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0xdb, 0xf8 # DB F8");
#endif
    return 0;
}
