// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_GROUP5_NOT_IMPLEMENTED]
// Can't run a group 5 operation the CPU does not implement.
// Note: FF /7 is reserved, so the hardware raises #UD too.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0xff, 0xf8 # FF /7");
#endif
    return 0;
}
