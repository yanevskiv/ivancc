// (Test) Compiler error: [ERR_TXT_OPERANDS_TOO_MANY]
// Can't give an instruction too many operands.

#ifdef __x86_64__
__asm__ ("mov %rax, %rbx, %rcx");
#endif
