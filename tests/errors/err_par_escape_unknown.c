// (Test) Compiler error: [ERR_PAR_ESCAPE_UNKNOWN]
// Can't use an escape sequence C does not define.
// Note: gcc only warns, and reads `q`.

char c = '\q';
