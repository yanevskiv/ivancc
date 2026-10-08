// (Test) Status: 234
// A kill the program sends itself with signal 65, past the last, returns -EINVAL, which it exits with, on the machine and in the emulator.

int main(void)
{
#ifdef __x86_64__
    __asm__ (
        "mov $39, %rax # getpid\n"
        "syscall\n"
        "mov %rax, %rdi\n"
        "mov $65, %rsi\n"
        "mov $62, %rax # kill\n"
        "syscall\n"
        "mov %rax, %rdi\n"
        "mov $60, %rax # exit\n"
        "syscall\n"
    );
#endif
    return 1;
}
