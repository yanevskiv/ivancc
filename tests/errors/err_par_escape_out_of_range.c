// (Test) Compiler error: [ERR_PAR_ESCAPE_OUT_OF_RANGE]
// Can't have a hex escape too large for a `char`.
// Note: gcc only warns.

char c = '\x100';
