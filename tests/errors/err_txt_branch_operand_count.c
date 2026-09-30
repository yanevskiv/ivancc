// (Test) Compiler error: [ERR_TXT_BRANCH_OPERAND_COUNT]
// Can't give a branch anything but one operand.

#ifdef __x86_64__
__asm__ ("jmp");
#endif
