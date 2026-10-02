// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_GROUP1_NOT_IMPLEMENTED]
// Can't run a group 1 operation the CPU does not implement.
// Note: the hardware runs `orl $1, %eax`, so a ud2 after it kills the native
// run with SIGILL too.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0x81, 0xc8, 0x01, 0x00, 0x00, 0x00 # orl $1, %eax\n"
             ".byte 0x0f, 0x0b # ud2");
#endif
    return 0;
}
