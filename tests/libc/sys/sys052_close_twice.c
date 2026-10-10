// (Test) Status: 247
// A second close of a descriptor returns -EBADF, which the program exits with, on the machine and in the emulator.

#define SYS_OPEN  2
#define SYS_CLOSE 3
#define SYS_EXIT  60

#define O_WRONLY 01
#define O_CREAT  0100

#define MODE 0644

#define PATH "file"

long sys(long nr, long a, long b, long c);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx\n"
         "syscall\n"
         "ret");
#endif

int main(void)
{
    long fd = sys(SYS_OPEN, (long) PATH, O_WRONLY | O_CREAT, MODE);

    if (fd < 0 || sys(SYS_CLOSE, fd, 0, 0) != 0) {
        return 1;
    }
    sys(SYS_EXIT, sys(SYS_CLOSE, fd, 0, 0), 0, 0);
    return 2;
}
