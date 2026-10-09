// (Test) Status: 234
// A clock_gettime of clock 99, which Linux lacks, returns -EINVAL, which the program exits with, on the machine and in the emulator.

int main(void)
{
#ifdef __x86_64__
    __asm__ (
        "sub $16, %rsp\n"
        "mov %rsp, %rsi\n"
        "mov $99, %rdi\n"
        "mov $228, %rax # clock_gettime\n"
        "syscall\n"
        "mov %rax, %rdi\n"
        "mov $60, %rax # exit\n"
        "syscall\n"
    );
#endif
    return 1;
}
