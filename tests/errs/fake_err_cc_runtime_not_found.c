// (Test) Compiler error: [ERR_CC_RUNTIME_NOT_FOUND]
// Can't link without finding the runtime directory.

// Skipped: only a host where ivcc cannot find its own path raises it.
#error "[ERR_CC_RUNTIME_NOT_FOUND]"
