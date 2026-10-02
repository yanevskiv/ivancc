// (Test) Compiler error: [ERR_CPU_TWO_BYTE_NOT_IMPLEMENTED]
// Can't run a two-byte opcode the CPU does not implement.

// Note: the decoder refuses every opcode the CPU does not run, as
// ERR_CPU_OPCODE_NOT_DECODABLE, so no program reaches it.
#error "[ERR_CPU_TWO_BYTE_NOT_IMPLEMENTED]"
