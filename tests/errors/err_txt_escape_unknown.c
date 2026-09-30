// (Test) Compiler error: [ERR_TXT_ESCAPE_UNKNOWN]
// Can't use an escape sequence the assembler does not know.
// Note: GNU as accepts it silently, and reads `q`.

#ifdef __x86_64__
__asm__ (".data\n.ascii \"\\q\"");
#endif
