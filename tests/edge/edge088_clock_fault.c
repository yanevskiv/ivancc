// (Test) Status: 242
// A clock_gettime of CLOCK_REALTIME into a null pointer returns -EFAULT, which the program exits with, on the machine and in the emulator.

int main(void)
{
#ifdef __x86_64__
    __asm__ (
        "sub $16, %rsp\n"
        "mov $0, %rsi\n"
        "mov $0, %rdi\n"
        "mov $228, %rax # clock_gettime\n"
        "syscall\n"
        "mov %rax, %rdi\n"
        "mov $60, %rax # exit\n"
        "syscall\n"
    );
#endif
    return 1;
}
