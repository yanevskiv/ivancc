// (Test) Status: 0
// (Test) Cleanup: edge137_rename.tmp edge137_rename.2.tmp
// rename moves a file to a second name, and fails a file that is gone, on the machine and in the emulator.

#define SYS_OPEN   2
#define SYS_CLOSE  3
#define SYS_RENAME 82
#define SYS_UNLINK 87

#define O_RDONLY 00
#define O_WRONLY 01
#define O_CREAT  0100

#define MODE 0644

#define ENOENT 2

#define FROM "edge137_rename.tmp"
#define TO   "edge137_rename.2.tmp"

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
    long fd = sys(SYS_OPEN, (long) FROM, O_WRONLY | O_CREAT, MODE);

    if (fd < 0 || sys(SYS_CLOSE, fd, 0, 0) != 0) {
        return 1;
    }
    if (sys(SYS_RENAME, (long) FROM, (long) TO, 0) != 0) {
        return 2;
    }
    if (sys(SYS_OPEN, (long) FROM, O_RDONLY, 0) != -ENOENT) {
        return 3;
    }
    fd = sys(SYS_OPEN, (long) TO, O_RDONLY, 0);
    if (fd < 0 || sys(SYS_CLOSE, fd, 0, 0) != 0) {
        return 4;
    }
    if (sys(SYS_RENAME, (long) FROM, (long) TO, 0) != -ENOENT) {
        return 5;
    }
    return sys(SYS_UNLINK, (long) TO, 0, 0) == 0 ? 0 : 6;
}
