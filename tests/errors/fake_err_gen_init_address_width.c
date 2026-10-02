// (Test) Compiler error: [ERR_GEN_INIT_ADDRESS_WIDTH]
// Can't initialize an object narrower than a pointer with an address.

// Note: an address cast to a narrower type never folds, so the fold fails first.
#error "[ERR_GEN_INIT_ADDRESS_WIDTH]"
