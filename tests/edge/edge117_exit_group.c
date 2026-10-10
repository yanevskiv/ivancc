// (Test) Status: 42
// exit_group ends the program with its status, on the machine and in the emulator.

#define SYS_EXIT_GROUP 231

#define STATUS 42

long sys(long nr, long a);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi\n"
         "syscall\n"
         "ret");
#endif

int main(void)
{
    sys(SYS_EXIT_GROUP, STATUS);
    return 1;
}
