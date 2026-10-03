// (Test) Compiler error: [ERR_TXT_INSTRUCTION_NOT_KNOWN]
// bug036. The encoder took operands in a shape it has no form for as
// another shape, and assembled `mov %eax, %rcx` as `mov %eax, %ecx`. Now
// each form the encoder has parses its own operands, and a line no form
// parses is refused.

#ifdef __x86_64__
__asm__ ("mov %eax, %rcx");
#endif
