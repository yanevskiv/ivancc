// C source file for user-facing diagnostics.

#include <stdarg.h>
#include <stdlib.h>

#include "util/err.h"
#include "util/log.h"
#include "util/str.h"

// Every diagnostic's name and message format.
static const Err_Entry Err_Table[ERR_CODE_COUNT] = {
    [ERR_FILE_ACCESS]                   = { "ERR_FILE_ACCESS",                   "%s: %s" },

    [ERR_STR_SLICE_OUT_OF_RANGE]        = { "ERR_STR_SLICE_OUT_OF_RANGE",        "slice [%zu, %zu) of a string of %zu bytes" },

    [ERR_PP_COMMENT_UNTERMINATED]       = { "ERR_PP_COMMENT_UNTERMINATED",       "unterminated comment" },
    [ERR_PP_DIRECTIVE_UNKNOWN]          = { "ERR_PP_DIRECTIVE_UNKNOWN",          "invalid preprocessing directive #%.*s" },
    [ERR_PP_INCLUDE_MALFORMED]          = { "ERR_PP_INCLUDE_MALFORMED",          "#include expects \"FILENAME\" or <FILENAME>" },
    [ERR_PP_INCLUDE_NOT_FOUND]          = { "ERR_PP_INCLUDE_NOT_FOUND",          "include file '%s' not found" },
    [ERR_PP_INCLUDE_TOO_DEEP]           = { "ERR_PP_INCLUDE_TOO_DEEP",           "#include nested more than %d deep" },
    [ERR_PP_MACRO_NAME_MISSING]         = { "ERR_PP_MACRO_NAME_MISSING",         "macro names must be identifiers" },
    [ERR_PP_MACRO_REDEFINED]            = { "ERR_PP_MACRO_REDEFINED",            "'%.*s' redefined" },
    [ERR_PP_MACRO_PARAMS_MALFORMED]     = { "ERR_PP_MACRO_PARAMS_MALFORMED",     "invalid macro parameter list" },
    [ERR_PP_MACRO_PARAM_DUPLICATE]      = { "ERR_PP_MACRO_PARAM_DUPLICATE",      "duplicate macro parameter '%.*s'" },
    [ERR_PP_MACRO_UNTERMINATED]         = { "ERR_PP_MACRO_UNTERMINATED",         "unterminated argument list invoking macro '%s'" },
    [ERR_PP_MACRO_ARGS_COUNT]           = { "ERR_PP_MACRO_ARGS_COUNT",           "macro '%s' requires %zu arguments, but %zu given" },
    [ERR_PP_VA_ARGS_MISPLACED]          = { "ERR_PP_VA_ARGS_MISPLACED",          "__VA_ARGS__ can only appear in the expansion of a variadic macro" },
    [ERR_PP_STRINGIZE_NOT_PARAM]        = { "ERR_PP_STRINGIZE_NOT_PARAM",        "'#' is not followed by a macro parameter" },
    [ERR_PP_PASTE_AT_EDGE]              = { "ERR_PP_PASTE_AT_EDGE",              "'##' cannot appear at either end of a macro expansion" },
    [ERR_PP_PASTE_INVALID]              = { "ERR_PP_PASTE_INVALID",              "pasting \"%.*s\" and \"%.*s\" does not give a valid preprocessing token" },

    [ERR_LEX_UNEXPECTED_CHAR]           = { "ERR_LEX_UNEXPECTED_CHAR",           "unexpected character '%s'" },

    [ERR_PAR_SYNTAX]                    = { "ERR_PAR_SYNTAX",                    "%s" },
    [ERR_PAR_TEXT_TOO_LONG]             = { "ERR_PAR_TEXT_TOO_LONG",             "preprocessed text of %zu bytes is too long for the lexer" },
    [ERR_PAR_ESCAPE_OUT_OF_RANGE]       = { "ERR_PAR_ESCAPE_OUT_OF_RANGE",       "escape sequence out of range for a %zu-byte character" },
    [ERR_PAR_ESCAPE_HEX_EMPTY]          = { "ERR_PAR_ESCAPE_HEX_EMPTY",          "\\x used with no following hex digits" },
    [ERR_PAR_ESCAPE_UCN_INCOMPLETE]     = { "ERR_PAR_ESCAPE_UCN_INCOMPLETE",     "incomplete universal character name" },
    [ERR_PAR_ESCAPE_UNKNOWN]            = { "ERR_PAR_ESCAPE_UNKNOWN",            "unknown escape sequence '\\%c'" },
    [ERR_PAR_DECL_UNNAMED]              = { "ERR_PAR_DECL_UNNAMED",              "this declaration needs a name" },
    [ERR_PAR_ARRAY_OF_FUNCTIONS]        = { "ERR_PAR_ARRAY_OF_FUNCTIONS",        "an array of functions is not a type" },
    [ERR_PAR_ARRAY_LEN_NOT_CONSTANT]    = { "ERR_PAR_ARRAY_LEN_NOT_CONSTANT",    "an array length is not a constant" },
    [ERR_PAR_ARRAY_DECOR_NOT_PARAM]     = { "ERR_PAR_ARRAY_DECOR_NOT_PARAM",     "'static' and qualifiers in an array declarator are only allowed on a parameter" },
    [ERR_PAR_ARRAY_DECOR_NOT_OUTERMOST] = { "ERR_PAR_ARRAY_DECOR_NOT_OUTERMOST", "'static' and qualifiers are only allowed on a parameter's outermost array" },
    [ERR_PAR_ARRAY_STATIC_NO_LEN]       = { "ERR_PAR_ARRAY_STATIC_NO_LEN",       "'static' in an array declarator needs a length" },
    [ERR_PAR_FUNCTION_BAD_RETURN]       = { "ERR_PAR_FUNCTION_BAD_RETURN",       "a function cannot return a function or an array" },
    [ERR_PAR_KNR_NOT_PARAM]             = { "ERR_PAR_KNR_NOT_PARAM",             "'%s' is not a parameter of this function" },
    [ERR_PAR_KNR_UNDECLARED]            = { "ERR_PAR_KNR_UNDECLARED",            "parameter '%s' has no declaration" },
    [ERR_PAR_SPEC_REPEATED]             = { "ERR_PAR_SPEC_REPEATED",             "a type specifier is repeated" },
    [ERR_PAR_SPEC_TWO_TYPES]            = { "ERR_PAR_SPEC_TWO_TYPES",            "two or more data types in one declaration" },
    [ERR_PAR_SPEC_MISSING]              = { "ERR_PAR_SPEC_MISSING",              "a declaration needs a type specifier" },
    [ERR_PAR_SPEC_INVALID]              = { "ERR_PAR_SPEC_INVALID",              "these type specifiers do not name a type" },
    [ERR_PAR_VOID_SIGNED]               = { "ERR_PAR_VOID_SIGNED",               "'void' cannot be signed or unsigned" },
    [ERR_PAR_BOOL_SIGNED]               = { "ERR_PAR_BOOL_SIGNED",               "'_Bool' cannot be signed or unsigned" },
    [ERR_PAR_VA_ARG_AGGREGATE]          = { "ERR_PAR_VA_ARG_AGGREGATE",          "__builtin_va_arg of a struct, union or array is not supported" },
    [ERR_PAR_BITFIELD_NOT_CONSTANT]     = { "ERR_PAR_BITFIELD_NOT_CONSTANT",     "a bit-field width is not a constant" },
    [ERR_PAR_BITFIELD_NOT_INTEGER]      = { "ERR_PAR_BITFIELD_NOT_INTEGER",      "a bit-field must have an integer type" },
    [ERR_PAR_BITFIELD_NEGATIVE]         = { "ERR_PAR_BITFIELD_NEGATIVE",         "a bit-field width cannot be negative" },
    [ERR_PAR_BITFIELD_TOO_WIDE]         = { "ERR_PAR_BITFIELD_TOO_WIDE",         "a bit-field is wider than the type that holds it" },
    [ERR_PAR_BITFIELD_NAMED_ZERO]       = { "ERR_PAR_BITFIELD_NAMED_ZERO",       "a bit-field with a name cannot be zero bits wide" },
    [ERR_PAR_MEMBER_UNNAMED]            = { "ERR_PAR_MEMBER_UNNAMED",            "this member needs a name" },
    [ERR_PAR_TAG_REDEFINED]             = { "ERR_PAR_TAG_REDEFINED",             "redefinition of '%s'" },
    [ERR_PAR_TAG_WRONG_KIND]            = { "ERR_PAR_TAG_WRONG_KIND",            "'%s' was declared with a different aggregate keyword" },
    [ERR_PAR_ENUM_NOT_CONSTANT]         = { "ERR_PAR_ENUM_NOT_CONSTANT",         "enumerator '%s' is not a constant" },
    [ERR_PAR_DESIG_NOT_AGGREGATE]       = { "ERR_PAR_DESIG_NOT_AGGREGATE",       "'.%s' designates a member of something that is not a struct or union" },
    [ERR_PAR_DESIG_NO_MEMBER]           = { "ERR_PAR_DESIG_NO_MEMBER",           "no member named '%s' to initialize" },
    [ERR_PAR_DESIG_NOT_ARRAY]           = { "ERR_PAR_DESIG_NOT_ARRAY",           "an index designator needs an array" },
    [ERR_PAR_DESIG_OUT_OF_RANGE]        = { "ERR_PAR_DESIG_OUT_OF_RANGE",        "initializer index %ld is outside the array" },
    [ERR_PAR_INIT_TOO_MANY_ELEMENTS]    = { "ERR_PAR_INIT_TOO_MANY_ELEMENTS",    "too many initializers for an array of %d" },
    [ERR_PAR_INIT_TOO_MANY_MEMBERS]     = { "ERR_PAR_INIT_TOO_MANY_MEMBERS",     "too many initializers for '%s'" },
    [ERR_PAR_INIT_ARRAY_UNBRACED]       = { "ERR_PAR_INIT_ARRAY_UNBRACED",       "an array needs a braced initializer" },
    [ERR_PAR_INIT_EMPTY]                = { "ERR_PAR_INIT_EMPTY",                "an empty initializer list has nothing to assign" },
    [ERR_PAR_LITERAL_INCOMPLETE]        = { "ERR_PAR_LITERAL_INCOMPLETE",        "a compound literal of an incomplete type has no size" },
    [ERR_PAR_OBJECT_INCOMPLETE]         = { "ERR_PAR_OBJECT_INCOMPLETE",         "'%s' has an incomplete type" },
    [ERR_PAR_TYPEDEF_INITIALIZED]       = { "ERR_PAR_TYPEDEF_INITIALIZED",       "a typedef takes no initializer" },
    [ERR_PAR_UNDECLARED]                = { "ERR_PAR_UNDECLARED",                "use of undeclared identifier '%s'" },

    [ERR_AST_FLEXIBLE_IN_UNION]         = { "ERR_AST_FLEXIBLE_IN_UNION",         "a union cannot have a flexible array member" },
    [ERR_AST_FLEXIBLE_NOT_LAST]         = { "ERR_AST_FLEXIBLE_NOT_LAST",         "flexible array member '%s' must come last" },
    [ERR_AST_FLEXIBLE_ALONE]            = { "ERR_AST_FLEXIBLE_ALONE",            "a struct needs a member before a flexible array member" },
    [ERR_AST_MEMBER_INCOMPLETE]         = { "ERR_AST_MEMBER_INCOMPLETE",         "member '%s' has an incomplete type" },
    [ERR_AST_MEMBER_DUPLICATE]          = { "ERR_AST_MEMBER_DUPLICATE",          "duplicate member '%s'" },
    [ERR_AST_AGGREGATE_UNNAMED]         = { "ERR_AST_AGGREGATE_UNNAMED",         "an aggregate must declare at least one named member" },
    [ERR_AST_TOO_MANY_STRINGS]          = { "ERR_AST_TOO_MANY_STRINGS",          "too many string literals (max %d)" },

    [ERR_SEM_DIVISION_BY_ZERO]          = { "ERR_SEM_DIVISION_BY_ZERO",          "division by zero in a constant expression" },
    [ERR_SEM_CALL_NOT_FUNCTION]         = { "ERR_SEM_CALL_NOT_FUNCTION",         "called object is not a function or function pointer" },
    [ERR_SEM_ARGS_TOO_FEW]              = { "ERR_SEM_ARGS_TOO_FEW",              "too few arguments to %s: got %d, expected at least %d" },
    [ERR_SEM_ARGS_WRONG_COUNT]          = { "ERR_SEM_ARGS_WRONG_COUNT",          "wrong number of arguments to %s: got %d, expected %d" },
    [ERR_SEM_ADD_POINTERS]              = { "ERR_SEM_ADD_POINTERS",              "cannot add two pointers" },
    [ERR_SEM_SUB_POINTER_FROM_INT]      = { "ERR_SEM_SUB_POINTER_FROM_INT",      "cannot subtract a pointer from an integer" },
    [ERR_SEM_GOTO_UNDEFINED]            = { "ERR_SEM_GOTO_UNDEFINED",            "goto names an undefined label '%s'" },
    [ERR_SEM_CASE_DUPLICATE]            = { "ERR_SEM_CASE_DUPLICATE",            "duplicate case in switch" },
    [ERR_SEM_CASE_NOT_CONSTANT]         = { "ERR_SEM_CASE_NOT_CONSTANT",         "case label is not a constant" },
    [ERR_SEM_ADDRESS_NOT_LVALUE]        = { "ERR_SEM_ADDRESS_NOT_LVALUE",        "cannot take the address of this expression" },
    [ERR_SEM_ADDRESS_BITFIELD]          = { "ERR_SEM_ADDRESS_BITFIELD",          "cannot take the address of a bit-field" },
    [ERR_SEM_DEREF_NOT_POINTER]         = { "ERR_SEM_DEREF_NOT_POINTER",         "indirection requires a pointer operand" },
    [ERR_SEM_DEREF_VOID]                = { "ERR_SEM_DEREF_VOID",                "cannot dereference a pointer to void" },
    [ERR_SEM_DEREF_INCOMPLETE]          = { "ERR_SEM_DEREF_INCOMPLETE",          "cannot dereference a pointer to an incomplete type" },
    [ERR_SEM_MEMBER_NOT_AGGREGATE]      = { "ERR_SEM_MEMBER_NOT_AGGREGATE",      "request for member '%s' in something that is not a struct or union" },
    [ERR_SEM_MEMBER_INCOMPLETE]         = { "ERR_SEM_MEMBER_INCOMPLETE",         "'%s' is an incomplete type" },
    [ERR_SEM_MEMBER_UNKNOWN]            = { "ERR_SEM_MEMBER_UNKNOWN",            "no member named '%s' in '%s'" },
    [ERR_SEM_NOT_ASSIGNABLE]            = { "ERR_SEM_NOT_ASSIGNABLE",            "expression is not assignable" },
    [ERR_SEM_ASSIGN_ARRAY]              = { "ERR_SEM_ASSIGN_ARRAY",              "cannot assign to an array" },
    [ERR_SEM_ASSIGN_AGGREGATE_MISMATCH] = { "ERR_SEM_ASSIGN_AGGREGATE_MISMATCH", "cannot assign a value of a different struct or union type" },
    [ERR_SEM_VA_START_FIXED]            = { "ERR_SEM_VA_START_FIXED",            "__builtin_va_start outside a variadic function" },

    [ERR_GEN_NOT_LVALUE]                = { "ERR_GEN_NOT_LVALUE",                "not an lvalue" },
    [ERR_GEN_UNEXPECTED_OPASSIGN]       = { "ERR_GEN_UNEXPECTED_OPASSIGN",       "unexpected compound assignment %d" },
    [ERR_GEN_UNEXPECTED_EXPR]           = { "ERR_GEN_UNEXPECTED_EXPR",           "unexpected node kind %d" },
    [ERR_GEN_UNEXPECTED_STMT]           = { "ERR_GEN_UNEXPECTED_STMT",           "unexpected statement kind %d" },
    [ERR_GEN_BREAK_OUTSIDE_LOOP]        = { "ERR_GEN_BREAK_OUTSIDE_LOOP",        "break outside a loop" },
    [ERR_GEN_CONTINUE_OUTSIDE_LOOP]     = { "ERR_GEN_CONTINUE_OUTSIDE_LOOP",     "continue outside a loop" },
    [ERR_GEN_INIT_TOO_LARGE]            = { "ERR_GEN_INIT_TOO_LARGE",            "initializer for '%s' is larger than it is" },
    [ERR_GEN_INIT_ADDRESS_WIDTH]        = { "ERR_GEN_INIT_ADDRESS_WIDTH",        "initializer for '%s' needs a pointer to hold an address" },
    [ERR_GEN_INIT_TOO_MANY_ADDRESSES]   = { "ERR_GEN_INIT_TOO_MANY_ADDRESSES",   "initializer for '%s' holds more addresses than %d" },
    [ERR_GEN_INIT_NOT_CONSTANT]         = { "ERR_GEN_INIT_NOT_CONSTANT",         "initializer for '%s' is not a constant" },

    [ERR_LINK_MULTIPLE_DEFINITION]      = { "ERR_LINK_MULTIPLE_DEFINITION",      "multiple definition of '%s'" },
    [ERR_LINK_OBJECT_UNREADABLE]        = { "ERR_LINK_OBJECT_UNREADABLE",        "cannot read object '%s'" },
    [ERR_LINK_UNDEFINED_SYMBOL]         = { "ERR_LINK_UNDEFINED_SYMBOL",         "undefined symbol '%s'" },
    [ERR_LINK_UNDEFINED_ENTRY]          = { "ERR_LINK_UNDEFINED_ENTRY",          "undefined entry symbol '%s'" },
    [ERR_LINK_UNSUPPORTED_RELOCATION]   = { "ERR_LINK_UNSUPPORTED_RELOCATION",   "unsupported relocation type %u" },

    [ERR_LOAD_NOT_ELF]                  = { "ERR_LOAD_NOT_ELF",                  "not an ELF file: '%s'" },
    [ERR_LOAD_NOT_ELF64_LSB]            = { "ERR_LOAD_NOT_ELF64_LSB",            "not a 64-bit little-endian ELF file: '%s'" },
    [ERR_LOAD_NOT_EXECUTABLE]           = { "ERR_LOAD_NOT_EXECUTABLE",           "not an executable: '%s'" },
    [ERR_LOAD_NO_SEGMENTS]              = { "ERR_LOAD_NO_SEGMENTS",              "no loadable segments in '%s'" },
    [ERR_LOAD_SEGMENT_TRUNCATED]        = { "ERR_LOAD_SEGMENT_TRUNCATED",        "segment runs past the end of '%s'" },

    [ERR_TXT_QUAD_NOT_ADDRESS]          = { "ERR_TXT_QUAD_NOT_ADDRESS",          "'%s' is not an address a .quad can hold" },
    [ERR_TXT_MNEMONIC_MALFORMED]        = { "ERR_TXT_MNEMONIC_MALFORMED",        "bad mnemonic in '%s'" },
    [ERR_TXT_MNEMONIC_UNKNOWN]          = { "ERR_TXT_MNEMONIC_UNKNOWN",          "unknown mnemonic '%.*s'" },
    [ERR_TXT_OPERANDS_TOO_MANY]         = { "ERR_TXT_OPERANDS_TOO_MANY",         "too many operands in '%s'" },
    [ERR_TXT_OPERAND_NOT_INDIRECT]      = { "ERR_TXT_OPERAND_NOT_INDIRECT",      "'%s' takes no indirect operand" },
    [ERR_TXT_OPERAND_MALFORMED]         = { "ERR_TXT_OPERAND_MALFORMED",         "bad operand '%s'" },
    [ERR_TXT_BRANCH_OPERAND_COUNT]      = { "ERR_TXT_BRANCH_OPERAND_COUNT",      "'%s' takes one operand" },
    [ERR_TXT_BRANCH_NOT_LABEL]          = { "ERR_TXT_BRANCH_NOT_LABEL",          "'%s' needs a label" },
    [ERR_TXT_STRING_UNTERMINATED]       = { "ERR_TXT_STRING_UNTERMINATED",       "missing closing quote in '%s'" },
    [ERR_TXT_ESCAPE_UNKNOWN]            = { "ERR_TXT_ESCAPE_UNKNOWN",            "unknown escape sequence '\\%c'" },

    [ERR_CC_ARCH_UNSUPPORTED]           = { "ERR_CC_ARCH_UNSUPPORTED",           "unsupported architecture '%s' (only %s is supported)" },
    [ERR_CC_RUNTIME_NOT_FOUND]          = { "ERR_CC_RUNTIME_NOT_FOUND",          "cannot locate the runtime directory; pass -B DIR" },
    [ERR_CC_OPTION_UNSUPPORTED]         = { "ERR_CC_OPTION_UNSUPPORTED",         "option '%s' is not supported yet" },
    [ERR_CC_STD_UNSUPPORTED]            = { "ERR_CC_STD_UNSUPPORTED",            "unsupported standard '%s' (only %s is supported)" },

    [ERR_LD_PLACE_MALFORMED]            = { "ERR_LD_PLACE_MALFORMED",            "malformed -place (expected SEC@ADDR): '%s'" },

    [ERR_EMU_ARCH_UNSUPPORTED]          = { "ERR_EMU_ARCH_UNSUPPORTED",          "unsupported architecture '%s' (only %s is supported)" },
    [ERR_EMU_NOT_X86_64]                = { "ERR_EMU_NOT_X86_64",                "'%s' is not an x86_64 executable" }
};

// The code of the last error raised.
static Err_Code Err_CurStatus = ERR_SUCCESS;

// Print an error and exit.
void Err_Raise(Err_Code code, ...)
{
    va_list ap;

    Err_CurStatus = code;
    va_start(ap, code);
    Err_ShowVa(LOG_SEVERITY_ERROR, LOG_LINE_NONE, code, ap);
    va_end(ap);
    exit(1);
}

// Print an error at a line and exit.
void Err_RaiseAt(uint32_t line, Err_Code code, ...)
{
    va_list ap;

    Err_CurStatus = code;
    va_start(ap, code);
    Err_ShowVa(LOG_SEVERITY_ERROR, line, code, ap);
    va_end(ap);
    exit(1);
}

// Print a warning at a line.
void Err_WarnAt(uint32_t line, Err_Code code, ...)
{
    va_list ap;

    va_start(ap, code);
    Err_ShowVa(LOG_SEVERITY_WARNING, line, code, ap);
    va_end(ap);
}

// Print one diagnostic from an argument list.
void Err_ShowVa(Log_Severity severity, uint32_t line, Err_Code code, va_list ap)
{
    if (code <= ERR_SUCCESS || code >= ERR_CODE_COUNT || ! Err_Table[code].ee_format) {
        Log_ShowError("diagnostic code %d has no entry", (int) code);
    }

    char *message = Str_FormatVa(Err_Table[code].ee_format, ap);

    Log_Show(severity, line, "%s [%s]", message, Err_Table[code].ee_name);
    Str_Free(message);
}

// Return the code of the last error raised.
Err_Code Err_Status(void)
{
    return Err_CurStatus;
}

// Return the message format of a code.
const char *Err_Message(Err_Code code)
{
    return Err_Table[code].ee_format;
}
