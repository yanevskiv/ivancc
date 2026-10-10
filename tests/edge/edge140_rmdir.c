// (Test) Status: 0
// (Test) Cleanup: edge140_rmdir.tmp
// rmdir removes an empty directory once, and fails it gone, a file, the current directory, its parent and an unmapped path, on the machine and in the emulator.

#define SYS_MKDIR 83
#define SYS_RMDIR 84

#define DIR_MODE 0755

#define ENOENT    2
#define EFAULT    14
#define ENOTDIR   20
#define EINVAL    22
#define ENOTEMPTY 39

#define UNMAPPED 8

#define PATH "edge140_rmdir.tmp"
#define FILE "edge140_rmdir.c"

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
    if (sys(SYS_MKDIR, (long) PATH, DIR_MODE, 0) != 0 || sys(SYS_RMDIR, (long) PATH, 0, 0) != 0) {
        return 1;
    }
    if (sys(SYS_RMDIR, (long) PATH, 0, 0) != -ENOENT) {
        return 2;
    }
    if (sys(SYS_RMDIR, (long) FILE, 0, 0) != -ENOTDIR) {
        return 3;
    }
    if (sys(SYS_RMDIR, (long) ".", 0, 0) != -EINVAL) {
        return 4;
    }
    if (sys(SYS_RMDIR, (long) "..", 0, 0) != -ENOTEMPTY) {
        return 5;
    }
    if (sys(SYS_RMDIR, UNMAPPED, 0, 0) != -EFAULT) {
        return 6;
    }
    return 0;
}
