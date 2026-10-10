// (Test) Status: 0
// lstat fails a missing path, a file as parent, an unmapped path and an unmapped buffer, the path's error first, on the machine and in the emulator.

#define SYS_OPEN  2
#define SYS_CLOSE 3
#define SYS_LSTAT 6

#define O_WRONLY 01
#define O_CREAT  0100

#define MODE 0644

#define ENOENT  2
#define EFAULT  14
#define ENOTDIR 20

#define UNMAPPED 8

#define MISSING "missing"
#define NOT_DIR "not_a_dir"
#define UNDER   "not_a_dir/inner"

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
    long fd = sys(SYS_OPEN, (long) NOT_DIR, O_WRONLY | O_CREAT, MODE);
    unsigned long buf[18];

    if (fd < 0 || sys(SYS_CLOSE, fd, 0, 0) != 0) {
        return 1;
    }
    if (sys(SYS_LSTAT, (long) MISSING, (long) buf, 0) != -ENOENT) {
        return 2;
    }
    if (sys(SYS_LSTAT, (long) UNDER, (long) buf, 0) != -ENOTDIR) {
        return 3;
    }
    if (sys(SYS_LSTAT, UNMAPPED, (long) buf, 0) != -EFAULT) {
        return 4;
    }
    if (sys(SYS_LSTAT, (long) NOT_DIR, UNMAPPED, 0) != -EFAULT) {
        return 5;
    }
    return sys(SYS_LSTAT, (long) MISSING, UNMAPPED, 0) == -ENOENT ? 0 : 6;
}
