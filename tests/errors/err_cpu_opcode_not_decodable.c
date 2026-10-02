// (Test) Status: 132
// (Test) Emulator error: [ERR_CPU_OPCODE_NOT_DECODABLE]
// Can't run an instruction that does not decode.

int main(void)
{
#ifdef __x86_64__
    __asm__ (".byte 0x06 # push %es, invalid in 64-bit mode");
#endif
    return 0;
}
