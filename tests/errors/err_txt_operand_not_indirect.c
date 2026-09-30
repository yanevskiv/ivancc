// (Test) Compiler error: [ERR_TXT_OPERAND_NOT_INDIRECT]
// Can't give an indirect operand to an instruction that takes none.

#ifdef __x86_64__
__asm__ ("mov *%rax, %rbx");
#endif
