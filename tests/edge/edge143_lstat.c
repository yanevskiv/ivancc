// (Test) Status: 0
// (Test) Cleanup: edge143_lstat.tmp
// lstat fills the 144 bytes of Linux's struct stat and no more, with a file's type, links and size, a directory's type and a link's own, on the machine and in the emulator.

#define SYS_WRITE  1
#define SYS_OPEN   2
#define SYS_CLOSE  3
#define SYS_LSTAT  6
#define SYS_UNLINK 87

#define O_WRONLY 01
#define O_CREAT  0100
#define O_EXCL   0200

#define MODE 0644

#define S_IFMT  0170000
#define S_IFREG 0100000
#define S_IFDIR 0040000
#define S_IFLNK 0120000

#define STAT_WORDS 18
#define NLINK_WORD 2
#define MODE_WORD  3
#define SIZE_WORD  6
#define MODE_MASK  0xFFFFFFFFUL
#define CANARY     0xAAAAAAAAAAAAAAAAUL

#define PATH "edge143_lstat.tmp"
#define TEXT "hello"
#define SIZE 5

long sys(long nr, long a, long b, long c);

#ifdef __x86_64__
__asm__ (".text\n"
         ".globl sys\n"
         "sys: movq %rdi, %rax; movq %rsi, %rdi; movq %rdx, %rsi; movq %rcx, %rdx\n"
         "syscall\n"
         "ret");
#endif

unsigned long buf[24];

// Return the type lstat gives path, or 0 when it fails.
unsigned long type_of(const char *path)
{
    if (sys(SYS_LSTAT, (long) path, (long) buf, 0) != 0) {
        return 0;
    }
    return (buf[MODE_WORD] & MODE_MASK) & S_IFMT;
}

int main(void)
{
    long fd = sys(SYS_OPEN, (long) PATH, O_WRONLY | O_CREAT | O_EXCL, MODE);

    if (fd < 0 || sys(SYS_WRITE, fd, (long) TEXT, SIZE) != SIZE || sys(SYS_CLOSE, fd, 0, 0) != 0) {
        return 1;
    }
    for (int i = 0; i < (int) (sizeof(buf) / sizeof(buf[0])); i++) {
        buf[i] = CANARY;
    }
    if (type_of(PATH) != S_IFREG) {
        return 2;
    }
    for (int i = STAT_WORDS; i < (int) (sizeof(buf) / sizeof(buf[0])); i++) {
        if (buf[i] != CANARY) {
            return 3;
        }
    }
    if (buf[NLINK_WORD] != 1 || buf[SIZE_WORD] != SIZE) {
        return 4;
    }
    if (type_of(".") != S_IFDIR) {
        return 5;
    }
    if (type_of("/proc/self") != S_IFLNK) {
        return 6;
    }
    return sys(SYS_UNLINK, (long) PATH, 0, 0) == 0 ? 0 : 7;
}
