// (Test) Compiler error: [ERR_TXT_DATA_NOT_KNOWN]
// Can't give a data directive an item that is neither a number nor, in
// `.quad`, an address.

#ifdef __x86_64__
__asm__ (".data\n.quad foo+bar");
#endif
