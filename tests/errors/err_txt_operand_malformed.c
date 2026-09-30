// (Test) Compiler error: [ERR_TXT_OPERAND_MALFORMED]
// Can't assemble a malformed operand.

#ifdef __x86_64__
__asm__ ("mov %foo, %rax");
#endif
