// (Test) Compiler error: [ERR_TXT_INSTRUCTION_NOT_KNOWN]
// Can't assemble an instruction the encoder has no form for.

#ifdef __x86_64__
__asm__ ("addq (%rax), (%rbx)");
#endif
