// (Test) Compiler error: [ERR_PAR_ESCAPE_UCN_NOT_COMPLETE]
// Can't have a `\u` escape with fewer than four hex digits.

char *s = "\u12";
