// (Test) Compiler error: [ERR_TXT_DIRECTIVE_NOT_KNOWN]
// Can't read a directive the assembler does not know, or its arguments.

#ifdef __x86_64__
__asm__ (".frob 1");
#endif
