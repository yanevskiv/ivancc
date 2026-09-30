// (Test) Compiler error: [ERR_PAR_ASM_WIDE_STRING]
// Can't write an asm template as a wide string.

#ifdef __x86_64__
__asm__ (L"");
#endif
