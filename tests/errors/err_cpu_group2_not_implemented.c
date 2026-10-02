// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_GROUP2_NOT_IMPLEMENTED]
// Can't run a group 2 operation the CPU does not implement.
// Note: the hardware runs `roll %cl, %eax`, so a ud2 after it kills the native
// run with SIGILL too.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0xd3, 0xc0 # roll %cl, %eax\n"
             ".byte 0x0f, 0x0b # ud2");
#endif
    return 0;
}
