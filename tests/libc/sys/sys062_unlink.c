// (Test) Status: 0
// unlink removes a file once, and fails it gone, a directory and an unmapped path, on the machine and in the emulator.

#define SYS_OPEN   2
#define SYS_CLOSE  3
#define SYS_UNLINK 87

#define O_WRONLY 01
#define O_CREAT  0100

#define MODE 0644

#define ENOENT 2
#define EFAULT 14
#define EISDIR 21

#define UNMAPPED 8

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
    if (sys(SYS_UNLINK, (long) PATH, 0, 0) != 0) {
        return 2;
    }
    if (sys(SYS_UNLINK, (long) PATH, 0, 0) != -ENOENT) {
        return 3;
    }
    if (sys(SYS_UNLINK, (long) ".", 0, 0) != -EISDIR) {
        return 4;
    }
    if (sys(SYS_UNLINK, UNMAPPED, 0, 0) != -EFAULT) {
        return 5;
    }
    return 0;
}
