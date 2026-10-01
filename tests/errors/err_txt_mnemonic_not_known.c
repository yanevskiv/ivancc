// (Test) Compiler error: [ERR_TXT_MNEMONIC_NOT_KNOWN]
// Can't assemble a mnemonic the assembler does not know.

#ifdef __x86_64__
__asm__ ("frob %rax");
#endif
