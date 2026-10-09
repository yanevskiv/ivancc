// (Test) Status: 0
// A clock_gettime of CLOCK_PROCESS_CPUTIME_ID into the stack returns 0, which the program exits with, on the machine and in the emulator.

int main(void)
{
#ifdef __x86_64__
    __asm__ (
        "sub $16, %rsp\n"
        "mov %rsp, %rsi\n"
        "mov $2, %rdi\n"
        "mov $228, %rax # clock_gettime\n"
        "syscall\n"
        "mov %rax, %rdi\n"
        "mov $60, %rax # exit\n"
        "syscall\n"
    );
#endif
    return 1;
}
