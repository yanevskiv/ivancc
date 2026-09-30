// (Test) Compiler error: [ERR_GEN_NOT_LVALUE]
// Can't take the address of a value that is not an lvalue.
// Note: sem refuses every other non-lvalue first. Only a bug reaches it, as
// `__builtin_va_arg(ap, struct S).a` does.

#error "[ERR_GEN_NOT_LVALUE]"
