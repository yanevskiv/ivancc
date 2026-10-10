// (Test) Status: 0
// A kill the program sends itself with SIGCHLD, ignored by default, returns 0, which it exits with, on the machine and in the emulator.

int main(void)
{
#ifdef __x86_64__
    __asm__ (
        "mov $39, %rax # getpid\n"
        "syscall\n"
        "mov %rax, %rdi\n"
        "mov $17, %rsi # SIGCHLD\n"
        "mov $62, %rax # kill\n"
        "syscall\n"
        "mov %rax, %rdi\n"
        "mov $60, %rax # exit\n"
        "syscall\n"
    );
#endif
    return 1;
}
