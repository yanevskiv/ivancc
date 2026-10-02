// (Test) Compiler error: [ERR_CPU_SSE_SINGLE_NOT_IMPLEMENTED]
// Can't run a scalar single SSE operation the CPU does not implement.

// Masked: the decoder takes only the SSE operations the CPU runs, and
// refuses the rest as ERR_CPU_OPCODE_NOT_DECODABLE, so no program reaches it.
#error "[ERR_CPU_SSE_SINGLE_NOT_IMPLEMENTED]"
