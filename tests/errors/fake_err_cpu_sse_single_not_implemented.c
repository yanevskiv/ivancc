// (Test) Compiler error: [ERR_CPU_SSE_SINGLE_NOT_IMPLEMENTED]
// Can't run a scalar single SSE operation the CPU does not implement.

// Note: the decoder takes only the SSE operations the CPU runs, so no
// program reaches it.
#error "[ERR_CPU_SSE_SINGLE_NOT_IMPLEMENTED]"
