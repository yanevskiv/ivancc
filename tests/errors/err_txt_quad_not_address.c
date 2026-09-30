// (Test) Compiler error: [ERR_TXT_QUAD_NOT_ADDRESS]
// Can't give `.quad` an operand that is not an address.

#ifdef __x86_64__
__asm__ (".data\n.quad foo+bar");
#endif
