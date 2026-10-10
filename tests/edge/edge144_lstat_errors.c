// (Test) Status: 0
// lstat fails a missing path, a file as parent, an unmapped path and an unmapped buffer, the path's error first, on the machine and in the emulator.

#define SYS_LSTAT 6

#define ENOENT  2
#define EFAULT  14
#define ENOTDIR 20

#define UNMAPPED 8

#define FILE    "edge144_lstat_errors.c"
#define MISSING "edge144_lstat_errors.tmp"
#define UNDER   "edge144_lstat_errors.c/inner"

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
    unsigned long buf[18];

    if (sys(SYS_LSTAT, (long) MISSING, (long) buf, 0) != -ENOENT) {
        return 1;
    }
    if (sys(SYS_LSTAT, (long) UNDER, (long) buf, 0) != -ENOTDIR) {
        return 2;
    }
    if (sys(SYS_LSTAT, UNMAPPED, (long) buf, 0) != -EFAULT) {
        return 3;
    }
    if (sys(SYS_LSTAT, (long) FILE, UNMAPPED, 0) != -EFAULT) {
        return 4;
    }
    return sys(SYS_LSTAT, (long) MISSING, UNMAPPED, 0) == -ENOENT ? 0 : 5;
}
