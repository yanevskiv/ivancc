// (Test) Status: 0
// An ioctl TCGETS of a terminal fills the 36 bytes of Linux's struct termios and no more, on the machine and in the emulator.

#define SYS_OPEN  2
#define SYS_IOCTL 16

#define O_RDWR   02
#define O_NOCTTY 0400

#define TCGETS 0x5401
#define CREAD  0200

#define TERMIOS_SIZE 36
#define CFLAG_OFF    8
#define CANARY       0xAA

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
    long fd = sys(SYS_OPEN, (long) "/dev/ptmx", O_RDWR | O_NOCTTY, 0);
    unsigned char buf[64];

    if (fd < 0) {
        return 1;
    }
    for (int i = 0; i < (int) sizeof(buf); i++) {
        buf[i] = CANARY;
    }
    if (sys(SYS_IOCTL, fd, TCGETS, (long) buf) != 0) {
        return 2;
    }
    for (int i = TERMIOS_SIZE; i < (int) sizeof(buf); i++) {
        if (buf[i] != CANARY) {
            return 3;
        }
    }
    return buf[CFLAG_OFF] & CREAD ? 0 : 4;
}
