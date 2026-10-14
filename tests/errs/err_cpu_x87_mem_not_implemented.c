// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_X87_MEM_NOT_IMPLEMENTED]
// Can't run an x87 memory operation the CPU does not implement.
// Note: D9 /1 on memory is reserved, so the hardware raises #UD too.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0xd9, 0x0c, 0x24 # D9 /1 (%rsp)");
#endif
    return 0;
}
