// (Test) Status: 234
// A clock_gettime of clock 99 into a null pointer returns -EINVAL before -EFAULT, which the program exits with, on the machine and in the emulator.

int main(void)
{
#ifdef __x86_64__
    __asm__ (
        "sub $16, %rsp\n"
        "mov $0, %rsi\n"
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
