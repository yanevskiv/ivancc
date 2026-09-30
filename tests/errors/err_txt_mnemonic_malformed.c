// (Test) Compiler error: [ERR_TXT_MNEMONIC_MALFORMED]
// Can't assemble a line whose mnemonic is malformed.
// Note: the mnemonic is longer than any the assembler knows.

#ifdef __x86_64__
__asm__ ("abcdefghijabcdefghijabcdefghijabcdefghij %rax");
#endif
