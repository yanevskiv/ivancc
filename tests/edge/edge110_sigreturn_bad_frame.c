// (Test) Status: 139
// An rt_sigreturn with no frame under the stack ends the program by SIGSEGV, on the machine and in the emulator.

int main(void)
{
#ifdef __x86_64__
    __asm__ (
        "movq $64, %rsp\n"
        "movq $15, %rax # rt_sigreturn\n"
        "syscall\n"
    );
#endif
    return 1;
}
