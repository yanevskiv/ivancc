// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_GROUP15_NOT_IMPLEMENTED]
// Can't run a group 15 operation the CPU does not implement.
// Note: 0F AE /2 takes only memory, so the hardware raises #UD on 0F AE D0 too.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0x0f, 0xae, 0xd0 # 0F AE /2 on %eax");
#endif
    return 0;
}
