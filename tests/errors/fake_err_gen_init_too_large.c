// (Test) Compiler error: [ERR_GEN_INIT_TOO_LARGE]
// Can't initialize past the end of an object.

// Note: the parser refuses every oversized initializer first.
#error "[ERR_GEN_INIT_TOO_LARGE]"
