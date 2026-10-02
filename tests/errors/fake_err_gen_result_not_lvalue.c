// (Test) Compiler error: [ERR_GEN_RESULT_NOT_LVALUE]
// Can't take the address of an assignment, comma or conditional of scalar type.

// Note: sem refuses every such address first, so no program reaches it.
#error "[ERR_GEN_RESULT_NOT_LVALUE]"
