// (Test) Compiler error: [ERR_TXT_BRANCH_NOT_LABEL]
// Can't branch to anything but a label.

#ifdef __x86_64__
__asm__ ("jmp $1");
#endif
