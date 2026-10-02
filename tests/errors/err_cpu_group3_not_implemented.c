// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_GROUP3_NOT_IMPLEMENTED]
// Can't run a group 3 operation the CPU does not implement.
// Note: the hardware runs `mull %eax`, so a ud2 after it kills the native
// run with SIGILL too.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0xf7, 0xe0 # mull %eax\n"
             ".byte 0x0f, 0x0b # ud2");
#endif
    return 0;
}
