// (Test) Compiler error: [ERR_CPU_X87_NOT_IMPLEMENTED]
// Can't run an x87 opcode the CPU does not implement.

// Unreachable: the CPU hands the x87 step only D9, DB, DD, DE and DF, and
// each raises its own code for a form it lacks.
#error "[ERR_CPU_X87_NOT_IMPLEMENTED]"
