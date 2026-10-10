// (Test) Status: 143
// A kill the program sends itself ends it by that signal, SIGTERM, on the machine and in the emulator.

int main(void)
{
#ifdef __x86_64__
    __asm__ (
        "mov $39, %rax # getpid\n"
        "syscall\n"
        "mov %rax, %rdi\n"
        "mov $15, %rsi # SIGTERM\n"
        "mov $62, %rax # kill\n"
        "syscall\n"
    );
#endif
    return 1;
}
