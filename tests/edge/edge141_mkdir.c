// (Test) Status: 0
// (Test) Cleanup: edge141_mkdir.tmp
// mkdir makes a directory a file can be made in, and fails it made, a file, a missing parent, a file as parent and an unmapped path, on the machine and in the emulator.

#define SYS_OPEN   2
#define SYS_CLOSE  3
#define SYS_MKDIR  83
#define SYS_RMDIR  84
#define SYS_UNLINK 87

#define O_WRONLY 01
#define O_CREAT  0100

#define MODE     0644
#define DIR_MODE 0755

#define ENOENT  2
#define EFAULT  14
#define EEXIST  17
#define ENOTDIR 20

#define UNMAPPED 8

#define PATH    "edge141_mkdir.tmp"
#define INNER   "edge141_mkdir.tmp/inner"
#define MISSING "edge141_mkdir.2.tmp/inner"
#define FILE    "edge141_mkdir.c"
#define UNDER   "edge141_mkdir.c/inner"

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
    long fd = 0;

    if (sys(SYS_MKDIR, (long) PATH, DIR_MODE, 0) != 0) {
        return 1;
    }
    fd = sys(SYS_OPEN, (long) INNER, O_WRONLY | O_CREAT, MODE);
    if (fd < 0 || sys(SYS_CLOSE, fd, 0, 0) != 0 || sys(SYS_UNLINK, (long) INNER, 0, 0) != 0) {
        return 2;
    }
    if (sys(SYS_MKDIR, (long) PATH, DIR_MODE, 0) != -EEXIST) {
        return 3;
    }
    if (sys(SYS_MKDIR, (long) FILE, DIR_MODE, 0) != -EEXIST) {
        return 4;
    }
    if (sys(SYS_MKDIR, (long) MISSING, DIR_MODE, 0) != -ENOENT) {
        return 5;
    }
    if (sys(SYS_MKDIR, (long) UNDER, DIR_MODE, 0) != -ENOTDIR) {
        return 6;
    }
    if (sys(SYS_MKDIR, UNMAPPED, DIR_MODE, 0) != -EFAULT) {
        return 7;
    }
    return sys(SYS_RMDIR, (long) PATH, 0, 0) == 0 ? 0 : 8;
}
