// (Test) Compiler error: [ERR_TXT_STRING_NOT_TERMINATED]
// Can't leave a string without its closing quote.
// Note: GNU as only warns, and closes the string, but then fails on the
// lines gcc writes after the template.

#ifdef __x86_64__
__asm__ (".data\n.ascii \"abc");
#endif
