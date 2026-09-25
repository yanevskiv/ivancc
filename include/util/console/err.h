// C header file for user-facing diagnostics.

#ifndef ERR_H
#define ERR_H

// Standard headers.
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Project headers.
#include "util/console/log.h"

// Raise an error unless a condition holds.
#define Err_Assert(cond, ...)                     \
    do {                                          \
        if (! (cond)) {                           \
            Err_Raise(__VA_ARGS__);               \
        }                                         \
    } while (0)

// Raise an error at a line unless a condition holds.
#define Err_AssertAt(line, cond, ...)             \
    do {                                          \
        if (! (cond)) {                           \
            Err_RaiseAt((line), __VA_ARGS__);     \
        }                                         \
    } while (0)

// Diagnostic codes, one per distinct message.
typedef enum Err_Code Err_Code;
enum Err_Code {
    ERR_SUCCESS,

    ERR_FILE_ACCESS,                   // path, reason

    ERR_STR_SLICE_OUT_OF_RANGE,        // start, end, length

    ERR_PP_COMMENT_UNTERMINATED,
    ERR_PP_DIRECTIVE_UNKNOWN,          // name length, name
    ERR_PP_INCLUDE_MALFORMED,
    ERR_PP_INCLUDE_NOT_FOUND,          // name
    ERR_PP_INCLUDE_TOO_DEEP,           // limit
    ERR_PP_MACRO_NAME_MISSING,
    ERR_PP_MACRO_REDEFINED,            // name length, name
    ERR_PP_MACRO_PARAMS_MALFORMED,
    ERR_PP_MACRO_PARAM_DUPLICATE,      // name length, name
    ERR_PP_MACRO_UNTERMINATED,         // name
    ERR_PP_MACRO_ARGS_COUNT,           // name, required, given
    ERR_PP_VA_ARGS_MISPLACED,
    ERR_PP_STRINGIZE_NOT_PARAM,
    ERR_PP_PASTE_AT_EDGE,
    ERR_PP_PASTE_INVALID,              // left length, left, right length, right
    ERR_PP_COND_UNTERMINATED,          // directive length, directive
    ERR_PP_COND_WITHOUT_IF,            // directive length, directive
    ERR_PP_COND_AFTER_ELSE,            // directive length, directive
    ERR_PP_COND_EXTRA_TOKENS,          // directive length, directive
    ERR_PP_DEFINED_NAME_MISSING,
    ERR_PP_DEFINED_PAREN_MISSING,
    ERR_PP_EXPR_EMPTY,                 // directive length, directive
    ERR_PP_EXPR_VALUE_MISSING,
    ERR_PP_EXPR_TOKEN_INVALID,         // token length, token
    ERR_PP_EXPR_OPERATOR_MISSING,      // token length, token
    ERR_PP_EXPR_PAREN_MISSING,
    ERR_PP_EXPR_COLON_MISSING,
    ERR_PP_EXPR_DIVISION_BY_ZERO,
    ERR_PP_EXPR_FLOAT,
    ERR_PP_EXPR_SUFFIX_INVALID,        // suffix
    ERR_PP_EXPR_TOO_LARGE,

    ERR_LEX_UNEXPECTED_CHAR,           // character

    ERR_PAR_SYNTAX,                    // parser message
    ERR_PAR_TEXT_TOO_LONG,             // length
    ERR_PAR_ESCAPE_OUT_OF_RANGE,       // element size
    ERR_PAR_ESCAPE_HEX_EMPTY,
    ERR_PAR_ESCAPE_UCN_INCOMPLETE,
    ERR_PAR_ESCAPE_UNKNOWN,            // character
    ERR_PAR_DECL_UNNAMED,
    ERR_PAR_ARRAY_OF_FUNCTIONS,
    ERR_PAR_ARRAY_LEN_NOT_CONSTANT,
    ERR_PAR_ARRAY_DECOR_NOT_PARAM,
    ERR_PAR_ARRAY_DECOR_NOT_OUTERMOST,
    ERR_PAR_ARRAY_STATIC_NO_LEN,
    ERR_PAR_FUNCTION_BAD_RETURN,
    ERR_PAR_KNR_NOT_PARAM,             // parameter
    ERR_PAR_KNR_UNDECLARED,            // parameter
    ERR_PAR_SPEC_REPEATED,
    ERR_PAR_SPEC_TWO_TYPES,
    ERR_PAR_SPEC_MISSING,
    ERR_PAR_SPEC_INVALID,
    ERR_PAR_VOID_SIGNED,
    ERR_PAR_BOOL_SIGNED,
    ERR_PAR_VA_ARG_AGGREGATE,
    ERR_PAR_BITFIELD_NOT_CONSTANT,
    ERR_PAR_BITFIELD_NOT_INTEGER,
    ERR_PAR_BITFIELD_NEGATIVE,
    ERR_PAR_BITFIELD_TOO_WIDE,
    ERR_PAR_BITFIELD_NAMED_ZERO,
    ERR_PAR_MEMBER_UNNAMED,
    ERR_PAR_TAG_REDEFINED,             // tag
    ERR_PAR_TAG_WRONG_KIND,            // tag
    ERR_PAR_ENUM_NOT_CONSTANT,         // enumerator
    ERR_PAR_DESIG_NOT_AGGREGATE,       // member
    ERR_PAR_DESIG_NO_MEMBER,           // member
    ERR_PAR_DESIG_NOT_ARRAY,
    ERR_PAR_DESIG_OUT_OF_RANGE,        // index
    ERR_PAR_INIT_TOO_MANY_ELEMENTS,    // array length
    ERR_PAR_INIT_TOO_MANY_MEMBERS,     // type name
    ERR_PAR_INIT_ARRAY_UNBRACED,
    ERR_PAR_INIT_EMPTY,
    ERR_PAR_LITERAL_INCOMPLETE,
    ERR_PAR_OBJECT_INCOMPLETE,         // name
    ERR_PAR_TYPEDEF_INITIALIZED,
    ERR_PAR_UNDECLARED,                // identifier

    ERR_AST_FLEXIBLE_IN_UNION,
    ERR_AST_FLEXIBLE_NOT_LAST,         // member
    ERR_AST_FLEXIBLE_ALONE,
    ERR_AST_MEMBER_INCOMPLETE,         // member
    ERR_AST_MEMBER_DUPLICATE,          // member
    ERR_AST_AGGREGATE_UNNAMED,
    ERR_AST_TOO_MANY_STRINGS,          // limit

    ERR_SEM_DIVISION_BY_ZERO,
    ERR_SEM_CALL_NOT_FUNCTION,
    ERR_SEM_ARGS_TOO_FEW,              // callee, count given, count wanted
    ERR_SEM_ARGS_WRONG_COUNT,          // callee, count given, count wanted
    ERR_SEM_ADD_POINTERS,
    ERR_SEM_SUB_POINTER_FROM_INT,
    ERR_SEM_GOTO_UNDEFINED,            // label
    ERR_SEM_CASE_DUPLICATE,
    ERR_SEM_CASE_NOT_CONSTANT,
    ERR_SEM_ADDRESS_NOT_LVALUE,
    ERR_SEM_ADDRESS_BITFIELD,
    ERR_SEM_DEREF_NOT_POINTER,
    ERR_SEM_DEREF_VOID,
    ERR_SEM_DEREF_INCOMPLETE,
    ERR_SEM_MEMBER_NOT_AGGREGATE,      // member
    ERR_SEM_MEMBER_INCOMPLETE,         // type name
    ERR_SEM_MEMBER_UNKNOWN,            // member, type name
    ERR_SEM_NOT_ASSIGNABLE,
    ERR_SEM_ASSIGN_ARRAY,
    ERR_SEM_ASSIGN_AGGREGATE_MISMATCH,
    ERR_SEM_VA_START_FIXED,

    ERR_GEN_NOT_LVALUE,
    ERR_GEN_UNEXPECTED_OPASSIGN,       // operator kind
    ERR_GEN_UNEXPECTED_EXPR,           // node kind
    ERR_GEN_UNEXPECTED_STMT,           // node kind
    ERR_GEN_BREAK_OUTSIDE_LOOP,
    ERR_GEN_CONTINUE_OUTSIDE_LOOP,
    ERR_GEN_INIT_TOO_LARGE,            // name
    ERR_GEN_INIT_ADDRESS_WIDTH,        // name
    ERR_GEN_INIT_TOO_MANY_ADDRESSES,   // name, limit
    ERR_GEN_INIT_NOT_CONSTANT,         // name

    ERR_LINK_MULTIPLE_DEFINITION,      // symbol
    ERR_LINK_OBJECT_UNREADABLE,        // path
    ERR_LINK_UNDEFINED_SYMBOL,         // symbol
    ERR_LINK_UNDEFINED_ENTRY,          // symbol
    ERR_LINK_UNSUPPORTED_RELOCATION,   // relocation type

    ERR_LOAD_NOT_ELF,                  // path
    ERR_LOAD_NOT_ELF64_LSB,            // path
    ERR_LOAD_NOT_EXECUTABLE,           // path
    ERR_LOAD_NO_SEGMENTS,              // path
    ERR_LOAD_SEGMENT_TRUNCATED,        // path

    ERR_TXT_QUAD_NOT_ADDRESS,          // operand
    ERR_TXT_MNEMONIC_MALFORMED,        // line
    ERR_TXT_MNEMONIC_UNKNOWN,          // mnemonic length, mnemonic
    ERR_TXT_OPERANDS_TOO_MANY,         // line
    ERR_TXT_OPERAND_NOT_INDIRECT,      // line
    ERR_TXT_OPERAND_MALFORMED,         // operand
    ERR_TXT_BRANCH_OPERAND_COUNT,      // line
    ERR_TXT_BRANCH_NOT_LABEL,          // line
    ERR_TXT_STRING_UNTERMINATED,       // string
    ERR_TXT_ESCAPE_UNKNOWN,            // character

    ERR_CC_ARCH_UNSUPPORTED,           // architecture, supported architecture
    ERR_CC_RUNTIME_NOT_FOUND,
    ERR_CC_OPTION_UNSUPPORTED,         // option
    ERR_CC_STD_UNSUPPORTED,            // standard, supported standard

    ERR_LD_PLACE_MALFORMED,            // placement

    ERR_EMU_ARCH_UNSUPPORTED,          // architecture, supported architecture
    ERR_EMU_NOT_X86_64,                // path

    ERR_CODE_COUNT
};

// A diagnostic's name and message format.
typedef struct Err_Entry Err_Entry;
struct Err_Entry {
    const char *ee_name;
    const char *ee_format;
};

// Raising
void Err_Raise(Err_Code code, ...);
void Err_RaiseAt(uint32_t line, Err_Code code, ...);
void Err_WarnAt(uint32_t line, Err_Code code, ...);
void Err_ShowVa(Log_Severity severity, uint32_t line, Err_Code code, va_list ap);

// Inspection
Err_Code    Err_Status(void);
const char *Err_Message(Err_Code code);

#endif // ERR_H
