/*
 * C source file for user-facing diagnostics.
 *
 * Copyright (C) 2026 Ivan Janevski
 *
 * ivancc is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * ivancc is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ivancc.  If not, see <https://www.gnu.org/licenses/>.
 */

// Module header.
#include "util/console/err.h"

// Every diagnostic's level, name and message format.
static const Err_Entry Err_Table[ERR_CODE_COUNT] = {
    // What: Can't open, read or write a file
    // Args: [path, reason]
    [ERR_FILE_ACCESS] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_FILE_ACCESS",
        .ee_format = "%s: %s"
    },

    // What: Can't slice a string out of range
    // Args: [start, end, length]
    [ERR_STR_SLICE_OUT_OF_RANGE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_STR_SLICE_OUT_OF_RANGE",
        .ee_format = "slice [%zu, %zu) of a string of %zu bytes"
    },

    // What: Can't leave a comment open at the end of the file
    // Args: none
    [ERR_PP_COMMENT_UNTERMINATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_COMMENT_UNTERMINATED",
        .ee_format = "unterminated comment"
    },

    // What: Can't use a directive the preprocessor does not know
    // Args: [name length, name]
    [ERR_PP_DIRECTIVE_UNKNOWN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_DIRECTIVE_UNKNOWN",
        .ee_format = "invalid preprocessing directive #%.*s"
    },

    // What: Shouldn't put tokens after a directive that takes none
    // Args: [directive length, directive]
    [ERR_PP_EXTRA_TOKENS] = {
        .ee_level  = ERR_LEVEL_PEDANTIC,
        .ee_name   = "ERR_PP_EXTRA_TOKENS",
        .ee_format = "extra tokens at end of #%.*s directive"
    },

    // What: Can't #include anything but "FILE" or <FILE>
    // Args: none
    [ERR_PP_INCLUDE_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_INCLUDE_MALFORMED",
        .ee_format = "#include expects \"FILENAME\" or <FILENAME>"
    },

    // What: Can't #include a file that is not on the search path
    // Args: [name]
    [ERR_PP_INCLUDE_NOT_FOUND] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_INCLUDE_NOT_FOUND",
        .ee_format = "include file '%s' not found"
    },

    // What: Can't nest #include past the limit
    // Args: [limit]
    [ERR_PP_INCLUDE_TOO_DEEP] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_INCLUDE_TOO_DEEP",
        .ee_format = "#include nested more than %d deep"
    },

    // What: Can't name a macro with anything but an identifier
    // Args: none
    [ERR_PP_MACRO_NAME_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_MACRO_NAME_MISSING",
        .ee_format = "macro names must be identifiers"
    },

    // What: Shouldn't redefine a macro with a different body
    // Args: [name length, name]
    [ERR_PP_MACRO_REDEFINED] = {
        .ee_level  = ERR_LEVEL_PEDANTIC,
        .ee_name   = "ERR_PP_MACRO_REDEFINED",
        .ee_format = "'%.*s' redefined"
    },

    // What: Can't write a malformed macro parameter list
    // Args: none
    [ERR_PP_MACRO_PARAMS_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_MACRO_PARAMS_MALFORMED",
        .ee_format = "invalid macro parameter list"
    },

    // What: Can't give a macro two parameters with the same name
    // Args: [name length, name]
    [ERR_PP_MACRO_PARAM_DUPLICATE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_MACRO_PARAM_DUPLICATE",
        .ee_format = "duplicate macro parameter '%.*s'"
    },

    // What: Can't leave a macro call's argument list open
    // Args: [name]
    [ERR_PP_MACRO_UNTERMINATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_MACRO_UNTERMINATED",
        .ee_format = "unterminated argument list invoking macro '%s'"
    },

    // What: Can't call a macro with the wrong number of arguments
    // Args: [name, required, given]
    [ERR_PP_MACRO_ARGS_COUNT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_MACRO_ARGS_COUNT",
        .ee_format = "macro '%s' requires %zu arguments, but %zu given"
    },

    // What: Can't use `__VA_ARGS__` in a macro that is not variadic
    // Args: none
    [ERR_PP_VA_ARGS_MISPLACED] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PP_VA_ARGS_MISPLACED",
        .ee_format = "__VA_ARGS__ can only appear in the expansion of a variadic macro"
    },

    // What: Can't apply `#` to a name that is not a parameter
    // Args: none
    [ERR_PP_STRINGIZE_NOT_PARAM] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_STRINGIZE_NOT_PARAM",
        .ee_format = "'#' is not followed by a macro parameter"
    },

    // What: Can't start or end a replacement list with `##`
    // Args: none
    [ERR_PP_PASTE_AT_EDGE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_PASTE_AT_EDGE",
        .ee_format = "'##' cannot appear at either end of a macro expansion"
    },

    // What: Can't paste two tokens that do not form one token
    // Args: [left length, left, right length, right]
    [ERR_PP_PASTE_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_PASTE_INVALID",
        .ee_format = "pasting \"%.*s\" and \"%.*s\" does not give a valid preprocessing token"
    },

    // What: Can't leave an #if without its #endif
    // Args: [directive length, directive]
    [ERR_PP_COND_UNTERMINATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_COND_UNTERMINATED",
        .ee_format = "unterminated #%.*s"
    },

    // What: Can't have an #elif, #else or #endif without an #if
    // Args: [directive length, directive]
    [ERR_PP_COND_WITHOUT_IF] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_COND_WITHOUT_IF",
        .ee_format = "#%.*s without #if"
    },

    // What: Can't have an #elif or #else after #else
    // Args: [directive length, directive]
    [ERR_PP_COND_AFTER_ELSE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_COND_AFTER_ELSE",
        .ee_format = "#%.*s after #else"
    },

    // What: Can't apply `defined` to anything but a name
    // Args: none
    [ERR_PP_DEFINED_NAME_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_DEFINED_NAME_MISSING",
        .ee_format = "operator \"defined\" requires an identifier"
    },

    // What: Can't leave `defined(` without its `)`
    // Args: none
    [ERR_PP_DEFINED_PAREN_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_DEFINED_PAREN_MISSING",
        .ee_format = "missing ')' after \"defined\""
    },

    // What: Can't have an #if or #elif with no expression
    // Args: [directive length, directive]
    [ERR_PP_EXPR_EMPTY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_EMPTY",
        .ee_format = "#%.*s with no expression"
    },

    // What: Can't leave out a value in #if
    // Args: none
    [ERR_PP_EXPR_VALUE_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_VALUE_MISSING",
        .ee_format = "expected a value in preprocessor expression"
    },

    // What: Can't use a token in #if that has no value there
    // Args: [token length, token]
    [ERR_PP_EXPR_TOKEN_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_TOKEN_INVALID",
        .ee_format = "token \"%.*s\" is not valid in preprocessor expressions"
    },

    // What: Can't put two values side by side in #if
    // Args: [token length, token]
    [ERR_PP_EXPR_OPERATOR_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_OPERATOR_MISSING",
        .ee_format = "missing binary operator before token \"%.*s\""
    },

    // What: Can't leave a parenthesis in #if open
    // Args: none
    [ERR_PP_EXPR_PAREN_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_PAREN_MISSING",
        .ee_format = "missing ')' in expression"
    },

    // What: Can't have a `?` without its `:` in #if
    // Args: none
    [ERR_PP_EXPR_COLON_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_COLON_MISSING",
        .ee_format = "'?' without following ':'"
    },

    // What: Can't divide by zero in #if
    // Args: none
    [ERR_PP_EXPR_DIVISION_BY_ZERO] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_DIVISION_BY_ZERO",
        .ee_format = "division by zero in #if"
    },

    // What: Can't use a floating constant in #if
    // Args: none
    [ERR_PP_EXPR_FLOAT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_FLOAT",
        .ee_format = "floating constant in preprocessor expression"
    },

    // What: Can't give an integer in #if a suffix no integer takes
    // Args: [suffix]
    [ERR_PP_EXPR_SUFFIX_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_SUFFIX_INVALID",
        .ee_format = "invalid suffix \"%s\" on integer constant"
    },

    // What: Can't use an integer in #if too large for any type
    // Args: none
    [ERR_PP_EXPR_TOO_LARGE] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PP_EXPR_TOO_LARGE",
        .ee_format = "integer constant is too large for its type"
    },

    // What: Can't have a #line without a line number
    // Args: none
    [ERR_PP_LINE_NUMBER_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_LINE_NUMBER_MISSING",
        .ee_format = "#line expects a line number"
    },

    // What: Can't give #line a line number that is not a number
    // Args: [token length, token]
    [ERR_PP_LINE_NUMBER_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_LINE_NUMBER_INVALID",
        .ee_format = "\"%.*s\" after #line is not a positive integer"
    },

    // What: Can't #line to line 0 or past 2147483647
    // Args: none
    [ERR_PP_LINE_OUT_OF_RANGE] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PP_LINE_OUT_OF_RANGE",
        .ee_format = "line number out of range"
    },

    // What: Can't give #line a file name that is not a string literal
    // Args: [token length, token]
    [ERR_PP_LINE_NAME_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_LINE_NAME_INVALID",
        .ee_format = "\"%.*s\" is not a valid filename"
    },

    // What: Can't give _Pragma anything but a string literal
    // Args: none
    [ERR_PP_PRAGMA_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_PRAGMA_MALFORMED",
        .ee_format = "_Pragma takes a parenthesized string literal"
    },

    // What: Shouldn't use #pragma once in the file being compiled
    // Args: none
    [ERR_PP_ONCE_IN_MAIN_FILE] = {
        .ee_level  = ERR_LEVEL_WARNING,
        .ee_name   = "ERR_PP_ONCE_IN_MAIN_FILE",
        .ee_format = "#pragma once in main file"
    },

    // What: Can't compile past an #error
    // Args: [text]
    [ERR_PP_ERROR_DIRECTIVE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_ERROR_DIRECTIVE",
        .ee_format = "#error %s"
    },

    // What: Shouldn't reach a #warning
    // Args: [text]
    [ERR_PP_WARNING_DIRECTIVE] = {
        .ee_level  = ERR_LEVEL_WARNING,
        .ee_name   = "ERR_PP_WARNING_DIRECTIVE",
        .ee_format = "#warning %s"
    },

    // What: Can't use a character that starts no token
    // Args: [character]
    [ERR_LEX_UNEXPECTED_CHAR] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LEX_UNEXPECTED_CHAR",
        .ee_format = "unexpected character '%s'"
    },

    // What: Can't write what C's grammar does not allow
    // Args: [parser message]
    [ERR_PAR_SYNTAX] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SYNTAX",
        .ee_format = "%s"
    },

    // What: Can't lex more than 2 GiB of preprocessed text
    // Args: [length]
    [ERR_PAR_TEXT_TOO_LONG] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_TEXT_TOO_LONG",
        .ee_format = "preprocessed text of %zu bytes is too long for the lexer"
    },

    // What: Can't have an escape too large for its character type
    // Args: [element size]
    [ERR_PAR_ESCAPE_OUT_OF_RANGE] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_ESCAPE_OUT_OF_RANGE",
        .ee_format = "escape sequence out of range for a %zu-byte character"
    },

    // What: Can't have a `\x` escape without hex digits
    // Args: none
    [ERR_PAR_ESCAPE_HEX_EMPTY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ESCAPE_HEX_EMPTY",
        .ee_format = "\\x used with no following hex digits"
    },

    // What: Can't have a `\u` or `\U` escape with too few hex digits
    // Args: none
    [ERR_PAR_ESCAPE_UCN_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ESCAPE_UCN_INCOMPLETE",
        .ee_format = "incomplete universal character name"
    },

    // What: Can't use an escape sequence C does not define
    // Args: [character]
    [ERR_PAR_ESCAPE_UNKNOWN] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_ESCAPE_UNKNOWN",
        .ee_format = "unknown escape sequence '\\%c'"
    },

    // What: Can't have a declaration whose declarator names nothing
    // Args: none
    [ERR_PAR_DECL_UNNAMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_DECL_UNNAMED",
        .ee_format = "this declaration needs a name"
    },

    // What: Can't have an array of functions
    // Args: none
    [ERR_PAR_ARRAY_OF_FUNCTIONS] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_OF_FUNCTIONS",
        .ee_format = "an array of functions is not a type"
    },

    // What: Can't give an array a run-time length where it needs a constant
    // Args: none
    [ERR_PAR_ARRAY_LEN_NOT_CONSTANT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_LEN_NOT_CONSTANT",
        .ee_format = "an array length is not a constant"
    },

    // What: Can't have an array of zero or negative length
    // Args: none
    [ERR_PAR_ARRAY_LEN_NOT_POSITIVE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_LEN_NOT_POSITIVE",
        .ee_format = "an array length must be greater than zero"
    },

    // What: Can't put `static` or a qualifier on a non-parameter array
    // Args: none
    [ERR_PAR_ARRAY_DECOR_NOT_PARAM] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_DECOR_NOT_PARAM",
        .ee_format = "'static' and qualifiers in an array declarator are only allowed on a parameter"
    },

    // What: Can't put `static` or a qualifier on a parameter's inner array
    // Args: none
    [ERR_PAR_ARRAY_DECOR_NOT_OUTERMOST] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_DECOR_NOT_OUTERMOST",
        .ee_format = "'static' and qualifiers are only allowed on a parameter's outermost array"
    },

    // What: Can't write `[static]` without a length
    // Args: none
    [ERR_PAR_ARRAY_STATIC_NO_LEN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_STATIC_NO_LEN",
        .ee_format = "'static' in an array declarator needs a length"
    },

    // What: Can't use `[*]` outside a prototype's parameters
    // Args: none
    [ERR_PAR_VLA_STAR_NOT_PROTOTYPE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_VLA_STAR_NOT_PROTOTYPE",
        .ee_format = "'[*]' is allowed only in a prototype's parameters"
    },

    // What: Shouldn't leave a tentative array without a length
    // Args: [name]
    [ERR_PAR_ARRAY_ASSUMED_ONE] = {
        .ee_level  = ERR_LEVEL_WARNING,
        .ee_name   = "ERR_PAR_ARRAY_ASSUMED_ONE",
        .ee_format = "array '%s' assumed to have one element"
    },

    // What: Can't have a function return a function or an array
    // Args: none
    [ERR_PAR_FUNCTION_BAD_RETURN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_FUNCTION_BAD_RETURN",
        .ee_format = "a function cannot return a function or an array"
    },

    // What: Can't declare a name an old-style parameter list does not have
    // Args: [parameter]
    [ERR_PAR_KNR_NOT_PARAM] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_KNR_NOT_PARAM",
        .ee_format = "'%s' is not a parameter of this function"
    },

    // What: Can't leave an old-style parameter undeclared
    // Args: [parameter]
    [ERR_PAR_KNR_UNDECLARED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_KNR_UNDECLARED",
        .ee_format = "parameter '%s' has no declaration"
    },

    // What: Can't repeat a type specifier
    // Args: none
    [ERR_PAR_SPEC_REPEATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SPEC_REPEATED",
        .ee_format = "a type specifier is repeated"
    },

    // What: Can't name two types in one declaration
    // Args: none
    [ERR_PAR_SPEC_TWO_TYPES] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SPEC_TWO_TYPES",
        .ee_format = "two or more data types in one declaration"
    },

    // What: Can't declare without a type specifier
    // Args: none
    [ERR_PAR_SPEC_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SPEC_MISSING",
        .ee_format = "a declaration needs a type specifier"
    },

    // What: Can't combine type specifiers that name no type
    // Args: none
    [ERR_PAR_SPEC_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SPEC_INVALID",
        .ee_format = "these type specifiers do not name a type"
    },

    // What: Can't give one declaration two storage classes
    // Args: none
    [ERR_PAR_STORAGE_REPEATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_STORAGE_REPEATED",
        .ee_format = "a declaration takes at most one storage class"
    },

    // What: Can't put a storage class or `inline` where it is not allowed
    // Args: none
    [ERR_PAR_STORAGE_NOT_ALLOWED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_STORAGE_NOT_ALLOWED",
        .ee_format = "a storage class or 'inline' is not allowed here"
    },

    // What: Can't have a signed or unsigned `void`
    // Args: none
    [ERR_PAR_VOID_SIGNED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_VOID_SIGNED",
        .ee_format = "'void' cannot be signed or unsigned"
    },

    // What: Can't have a signed or unsigned `_Bool`
    // Args: none
    [ERR_PAR_BOOL_SIGNED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BOOL_SIGNED",
        .ee_format = "'_Bool' cannot be signed or unsigned"
    },

    // What: Can't have a signed or unsigned floating type
    // Args: none
    [ERR_PAR_FLOAT_SIGNED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_FLOAT_SIGNED",
        .ee_format = "a floating type cannot be signed or unsigned"
    },

    // What: Can't take an incomplete type or an array from `__builtin_va_arg`
    // Args: none
    [ERR_PAR_VA_ARG_TYPE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_VA_ARG_TYPE",
        .ee_format = "__builtin_va_arg needs a complete object type that is not an array"
    },

    // What: Can't have a bit-field width known only at run time
    // Args: none
    [ERR_PAR_BITFIELD_NOT_CONSTANT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BITFIELD_NOT_CONSTANT",
        .ee_format = "a bit-field width is not a constant"
    },

    // What: Can't have a bit-field of a type that is not an integer
    // Args: none
    [ERR_PAR_BITFIELD_NOT_INTEGER] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BITFIELD_NOT_INTEGER",
        .ee_format = "a bit-field must have an integer type"
    },

    // What: Can't have a bit-field of negative width
    // Args: none
    [ERR_PAR_BITFIELD_NEGATIVE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BITFIELD_NEGATIVE",
        .ee_format = "a bit-field width cannot be negative"
    },

    // What: Can't have a bit-field wider than its type
    // Args: none
    [ERR_PAR_BITFIELD_TOO_WIDE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BITFIELD_TOO_WIDE",
        .ee_format = "a bit-field is wider than the type that holds it"
    },

    // What: Can't have a named bit-field of zero width
    // Args: none
    [ERR_PAR_BITFIELD_NAMED_ZERO] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BITFIELD_NAMED_ZERO",
        .ee_format = "a bit-field with a name cannot be zero bits wide"
    },

    // What: Can't have a member whose declarator names nothing
    // Args: none
    [ERR_PAR_MEMBER_UNNAMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_MEMBER_UNNAMED",
        .ee_format = "this member needs a name"
    },

    // What: Can't define one tag twice in a scope
    // Args: [tag]
    [ERR_PAR_TAG_REDEFINED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_TAG_REDEFINED",
        .ee_format = "redefinition of '%s'"
    },

    // What: Can't use a tag with a different aggregate keyword
    // Args: [tag]
    [ERR_PAR_TAG_WRONG_KIND] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_TAG_WRONG_KIND",
        .ee_format = "'%s' was declared with a different aggregate keyword"
    },

    // What: Can't give an enumerator a value known only at run time
    // Args: [enumerator]
    [ERR_PAR_ENUM_NOT_CONSTANT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ENUM_NOT_CONSTANT",
        .ee_format = "enumerator '%s' is not a constant"
    },

    // What: Can't designate a member of something that is not a struct or union
    // Args: [member]
    [ERR_PAR_DESIG_NOT_AGGREGATE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_DESIG_NOT_AGGREGATE",
        .ee_format = "'.%s' designates a member of something that is not a struct or union"
    },

    // What: Can't designate a member the struct does not have
    // Args: [member]
    [ERR_PAR_DESIG_NO_MEMBER] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_DESIG_NO_MEMBER",
        .ee_format = "no member named '%s' to initialize"
    },

    // What: Can't designate an index of something that is not an array
    // Args: none
    [ERR_PAR_DESIG_NOT_ARRAY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_DESIG_NOT_ARRAY",
        .ee_format = "an index designator needs an array"
    },

    // What: Can't designate an index past the end of the array
    // Args: [index]
    [ERR_PAR_DESIG_OUT_OF_RANGE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_DESIG_OUT_OF_RANGE",
        .ee_format = "initializer index %ld is outside the array"
    },

    // What: Can't give an array more initializers than elements
    // Args: [array length]
    [ERR_PAR_INIT_TOO_MANY_ELEMENTS] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_INIT_TOO_MANY_ELEMENTS",
        .ee_format = "too many initializers for an array of %d"
    },

    // What: Can't give a struct more initializers than members
    // Args: [type name]
    [ERR_PAR_INIT_TOO_MANY_MEMBERS] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_INIT_TOO_MANY_MEMBERS",
        .ee_format = "too many initializers for '%s'"
    },

    // What: Can't initialize an array without braces
    // Args: none
    [ERR_PAR_INIT_ARRAY_UNBRACED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_INIT_ARRAY_UNBRACED",
        .ee_format = "an array needs a braced initializer"
    },

    // What: Can't initialize with empty braces
    // Args: none
    [ERR_PAR_INIT_EMPTY] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_INIT_EMPTY",
        .ee_format = "an empty initializer list has nothing to assign"
    },

    // What: Can't initialize an array with a string of another width
    // Args: [element size, character size]
    [ERR_PAR_INIT_STRING_WIDTH] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_INIT_STRING_WIDTH",
        .ee_format = "an array of %d-byte elements cannot take a string of %d-byte characters"
    },

    // What: Can't initialize an array with a longer string literal
    // Args: [array length]
    [ERR_PAR_INIT_STRING_TOO_LONG] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_INIT_STRING_TOO_LONG",
        .ee_format = "initializer string is too long for an array of %d"
    },

    // What: Can't have a compound literal of an incomplete type
    // Args: none
    [ERR_PAR_LITERAL_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_LITERAL_INCOMPLETE",
        .ee_format = "a compound literal of an incomplete type has no size"
    },

    // What: Can't define an object of an incomplete type
    // Args: [name]
    [ERR_PAR_OBJECT_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_OBJECT_INCOMPLETE",
        .ee_format = "'%s' has an incomplete type"
    },

    // What: Can't initialize one file-scope object twice
    // Args: [name]
    [ERR_PAR_OBJECT_REDEFINED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_OBJECT_REDEFINED",
        .ee_format = "redefinition of '%s'"
    },

    // What: Can't declare an object `static` and then without it
    // Args: [name]
    [ERR_PAR_OBJECT_LINKAGE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_OBJECT_LINKAGE",
        .ee_format = "'%s' is declared both static and not"
    },

    // What: Can't initialize a block-scope `extern`
    // Args: [name]
    [ERR_PAR_EXTERN_INITIALIZED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_EXTERN_INITIALIZED",
        .ee_format = "'%s' is extern inside a block and takes no initializer"
    },

    // What: Can't take the `sizeof` of an incomplete type
    // Args: none
    [ERR_PAR_SIZEOF_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SIZEOF_INCOMPLETE",
        .ee_format = "invalid application of 'sizeof' to an incomplete type"
    },

    // What: Can't initialize a typedef
    // Args: none
    [ERR_PAR_TYPEDEF_INITIALIZED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_TYPEDEF_INITIALIZED",
        .ee_format = "a typedef takes no initializer"
    },

    // What: Can't initialize a variable-length array
    // Args: [name]
    [ERR_PAR_VLA_INITIALIZED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_VLA_INITIALIZED",
        .ee_format = "variable-length array '%s' takes no initializer"
    },

    // What: Can't use a name that was never declared
    // Args: [identifier]
    [ERR_PAR_UNDECLARED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_UNDECLARED",
        .ee_format = "use of undeclared identifier '%s'"
    },

    // What: Can't give an object a function body
    // Args: [name]
    [ERR_PAR_BODY_NOT_FUNCTION] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BODY_NOT_FUNCTION",
        .ee_format = "'%s' is not a function and takes no body"
    },

    // What: Can't declare one name twice in a scope
    // Args: [name]
    [ERR_PAR_REDECLARED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_REDECLARED",
        .ee_format = "redeclaration of '%s' in the same scope"
    },

    // What: Can't declare one name with two different types
    // Args: [name]
    [ERR_PAR_CONFLICTING_TYPES] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_CONFLICTING_TYPES",
        .ee_format = "conflicting types for '%s'"
    },

    // What: Can't give one function two bodies
    // Args: [name]
    [ERR_PAR_FUNCTION_REDEFINED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_FUNCTION_REDEFINED",
        .ee_format = "redefinition of function '%s'"
    },

    // What: Can't have a flexible array member in a union
    // Args: none
    [ERR_AST_FLEXIBLE_IN_UNION] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_AST_FLEXIBLE_IN_UNION",
        .ee_format = "a union cannot have a flexible array member"
    },

    // What: Can't have a member after a flexible array member
    // Args: [member]
    [ERR_AST_FLEXIBLE_NOT_LAST] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_AST_FLEXIBLE_NOT_LAST",
        .ee_format = "flexible array member '%s' must come last"
    },

    // What: Can't have a flexible array member as a struct's only member
    // Args: none
    [ERR_AST_FLEXIBLE_ALONE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_AST_FLEXIBLE_ALONE",
        .ee_format = "a struct needs a member before a flexible array member"
    },

    // What: Can't have a member of an incomplete type
    // Args: [member]
    [ERR_AST_MEMBER_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_AST_MEMBER_INCOMPLETE",
        .ee_format = "member '%s' has an incomplete type"
    },

    // What: Can't have two members with the same name
    // Args: [member]
    [ERR_AST_MEMBER_DUPLICATE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_AST_MEMBER_DUPLICATE",
        .ee_format = "duplicate member '%s'"
    },

    // What: Can't have a struct or union with no named member
    // Args: none
    [ERR_AST_AGGREGATE_UNNAMED] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_AST_AGGREGATE_UNNAMED",
        .ee_format = "an aggregate must declare at least one named member"
    },

    // What: Can't divide by zero in a constant expression
    // Args: none
    [ERR_SEM_DIVISION_BY_ZERO] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_DIVISION_BY_ZERO",
        .ee_format = "division by zero in a constant expression"
    },

    // What: Can't call something that is not a function or a function pointer
    // Args: none
    [ERR_SEM_CALL_NOT_FUNCTION] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CALL_NOT_FUNCTION",
        .ee_format = "called object is not a function or function pointer"
    },

    // What: Can't call a variadic function with too few named arguments
    // Args: [callee, count given, count wanted]
    [ERR_SEM_ARGS_TOO_FEW] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ARGS_TOO_FEW",
        .ee_format = "too few arguments to %s: got %d, expected at least %d"
    },

    // What: Can't call a function with the wrong number of arguments
    // Args: [callee, count given, count wanted]
    [ERR_SEM_ARGS_WRONG_COUNT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ARGS_WRONG_COUNT",
        .ee_format = "wrong number of arguments to %s: got %d, expected %d"
    },

    // What: Can't add two pointers
    // Args: none
    [ERR_SEM_ADD_POINTERS] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ADD_POINTERS",
        .ee_format = "cannot add two pointers"
    },

    // What: Can't subtract a pointer from an integer
    // Args: none
    [ERR_SEM_SUB_POINTER_FROM_INT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_SUB_POINTER_FROM_INT",
        .ee_format = "cannot subtract a pointer from an integer"
    },

    // What: Can't goto a label that is never defined
    // Args: [label]
    [ERR_SEM_GOTO_UNDEFINED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_GOTO_UNDEFINED",
        .ee_format = "goto names an undefined label '%s'"
    },

    // What: Can't goto into the scope of a variable-length array
    // Args: [label]
    [ERR_SEM_GOTO_INTO_VARMOD_SCOPE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_GOTO_INTO_VARMOD_SCOPE",
        .ee_format = "goto '%s' jumps into the scope of a variably modified name"
    },

    // What: Can't have two case labels with the same value
    // Args: none
    [ERR_SEM_CASE_DUPLICATE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CASE_DUPLICATE",
        .ee_format = "duplicate case in switch"
    },

    // What: Can't jump from a switch into the scope of a variable-length array
    // Args: none
    [ERR_SEM_CASE_INTO_VARMOD_SCOPE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CASE_INTO_VARMOD_SCOPE",
        .ee_format = "switch jumps into the scope of a variably modified name"
    },

    // What: Can't have a case label known only at run time
    // Args: none
    [ERR_SEM_CASE_NOT_CONSTANT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CASE_NOT_CONSTANT",
        .ee_format = "case label is not a constant"
    },

    // What: Can't take the address of a value that is not an lvalue
    // Args: none
    [ERR_SEM_ADDRESS_NOT_LVALUE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ADDRESS_NOT_LVALUE",
        .ee_format = "cannot take the address of this expression"
    },

    // What: Can't take the address of a bit-field
    // Args: none
    [ERR_SEM_ADDRESS_BITFIELD] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ADDRESS_BITFIELD",
        .ee_format = "cannot take the address of a bit-field"
    },

    // What: Can't dereference something that is not a pointer
    // Args: none
    [ERR_SEM_DEREF_NOT_POINTER] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_DEREF_NOT_POINTER",
        .ee_format = "indirection requires a pointer operand"
    },

    // What: Can't dereference a `void *` for its value
    // Args: none
    [ERR_SEM_DEREF_VOID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_DEREF_VOID",
        .ee_format = "cannot dereference a pointer to void"
    },

    // What: Can't dereference a pointer to an incomplete type
    // Args: none
    [ERR_SEM_DEREF_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_DEREF_INCOMPLETE",
        .ee_format = "cannot dereference a pointer to an incomplete type"
    },

    // What: Can't take the `sizeof` of an expression of incomplete type
    // Args: none
    [ERR_SEM_SIZEOF_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_SIZEOF_INCOMPLETE",
        .ee_format = "invalid application of 'sizeof' to an incomplete type"
    },

    // What: Can't give an array a length that is not an integer
    // Args: none
    [ERR_SEM_ARRAY_LEN_NOT_INTEGER] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ARRAY_LEN_NOT_INTEGER",
        .ee_format = "an array length is not an integer"
    },

    // What: Can't access a member of something that is not a struct or union
    // Args: [member]
    [ERR_SEM_MEMBER_NOT_AGGREGATE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_MEMBER_NOT_AGGREGATE",
        .ee_format = "request for member '%s' in something that is not a struct or union"
    },

    // What: Can't access a member of an incomplete type
    // Args: [type name]
    [ERR_SEM_MEMBER_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_MEMBER_INCOMPLETE",
        .ee_format = "'%s' is an incomplete type"
    },

    // What: Can't access a member the struct or union does not have
    // Args: [member, type name]
    [ERR_SEM_MEMBER_UNKNOWN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_MEMBER_UNKNOWN",
        .ee_format = "no member named '%s' in '%s'"
    },

    // What: Can't assign to a value that is not an lvalue
    // Args: none
    [ERR_SEM_NOT_ASSIGNABLE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_NOT_ASSIGNABLE",
        .ee_format = "expression is not assignable"
    },

    // What: Can't assign to an array
    // Args: none
    [ERR_SEM_ASSIGN_ARRAY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ASSIGN_ARRAY",
        .ee_format = "cannot assign to an array"
    },

    // What: Can't assign to a `const` object
    // Args: none
    [ERR_SEM_ASSIGN_CONST] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ASSIGN_CONST",
        .ee_format = "cannot store to a read-only object or an array"
    },

    // What: Can't assign between incompatible types
    // Args: [context]
    [ERR_SEM_ASSIGN_INCOMPATIBLE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ASSIGN_INCOMPATIBLE",
        .ee_format = "incompatible types in %s"
    },

    // What: Can't drop a qualifier of the pointed-to type in a conversion
    // Args: [context]
    [ERR_SEM_ASSIGN_DISCARDS_QUALIFIER] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_SEM_ASSIGN_DISCARDS_QUALIFIER",
        .ee_format = "%s discards a qualifier of the pointed-to type"
    },

    // What: Can't use a `void` value
    // Args: none
    [ERR_SEM_VOID_VALUE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_VOID_VALUE",
        .ee_format = "a void value cannot be used"
    },

    // What: Can't choose between incompatible types in a conditional
    // Args: none
    [ERR_SEM_COND_MISMATCH] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_COND_MISMATCH",
        .ee_format = "the operands of '?:' have incompatible types"
    },

    // What: Can't return a value from a `void` function
    // Args: none
    [ERR_SEM_RETURN_VALUE_IN_VOID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_RETURN_VALUE_IN_VOID",
        .ee_format = "a function returning void cannot return a value"
    },

    // What: Can't return nothing from a function that returns a value
    // Args: none
    [ERR_SEM_RETURN_NO_VALUE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_RETURN_NO_VALUE",
        .ee_format = "a function returning a value needs one in every return"
    },

    // What: Can't use `__builtin_va_start` in a function that is not variadic
    // Args: none
    [ERR_SEM_VA_START_FIXED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_VA_START_FIXED",
        .ee_format = "__builtin_va_start outside a variadic function"
    },

    // What: Can't add a floating value to a pointer or subtract one from it
    // Args: none
    [ERR_SEM_POINTER_FLOATING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_POINTER_FLOATING",
        .ee_format = "pointer arithmetic needs an integer, not a floating operand"
    },

    // What: Can't apply an integer operator to a floating operand
    // Args: none
    [ERR_SEM_OPERAND_FLOATING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_OPERAND_FLOATING",
        .ee_format = "this operator needs integer operands, not floating ones"
    },

    // What: Can't cast between a floating type and a pointer
    // Args: none
    [ERR_SEM_CAST_FLOATING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CAST_FLOATING",
        .ee_format = "cannot convert between a floating type and a non-arithmetic one"
    },

    // What: Can't apply an arithmetic operator to what is not a number
    // Args: none
    [ERR_SEM_OPERAND_NOT_ARITHMETIC] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_OPERAND_NOT_ARITHMETIC",
        .ee_format = "this operator takes only arithmetic operands"
    },

    // What: Can't apply an integer operator to what is not an integer
    // Args: none
    [ERR_SEM_OPERAND_NOT_INTEGER] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_OPERAND_NOT_INTEGER",
        .ee_format = "this operator takes only integer operands"
    },

    // What: Can't test for truth what is not a number or a pointer
    // Args: none
    [ERR_SEM_OPERAND_NOT_SCALAR] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_OPERAND_NOT_SCALAR",
        .ee_format = "a value tested for truth must be a number or a pointer"
    },

    // What: Can't cast anything but a number or pointer to one or to `void`
    // Args: none
    [ERR_SEM_CAST_NOT_SCALAR] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CAST_NOT_SCALAR",
        .ee_format = "a cast converts only a number or a pointer, to one or to void"
    },

    // What: Can't take the address of a value that is not an lvalue
    // Args: none
    [ERR_GEN_NOT_LVALUE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_NOT_LVALUE",
        .ee_format = "not an lvalue"
    },

    // What: Can't generate code for a compound assignment gen does not know
    // Args: [operator kind]
    [ERR_GEN_UNEXPECTED_OPASSIGN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_UNEXPECTED_OPASSIGN",
        .ee_format = "unexpected compound assignment %d"
    },

    // What: Can't generate code for an expression node gen does not know
    // Args: [node kind]
    [ERR_GEN_UNEXPECTED_EXPR] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_UNEXPECTED_EXPR",
        .ee_format = "unexpected node kind %d"
    },

    // What: Can't generate code for a statement node gen does not know
    // Args: [node kind]
    [ERR_GEN_UNEXPECTED_STMT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_UNEXPECTED_STMT",
        .ee_format = "unexpected statement kind %d"
    },

    // What: Can't break outside a loop or a switch
    // Args: none
    [ERR_GEN_BREAK_OUTSIDE_LOOP] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_BREAK_OUTSIDE_LOOP",
        .ee_format = "break outside a loop"
    },

    // What: Can't continue outside a loop
    // Args: none
    [ERR_GEN_CONTINUE_OUTSIDE_LOOP] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_CONTINUE_OUTSIDE_LOOP",
        .ee_format = "continue outside a loop"
    },

    // What: Can't initialize past the end of an object
    // Args: [name]
    [ERR_GEN_INIT_TOO_LARGE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_INIT_TOO_LARGE",
        .ee_format = "initializer for '%s' is larger than it is"
    },

    // What: Can't initialize an object narrower than a pointer with an address
    // Args: [name]
    [ERR_GEN_INIT_ADDRESS_WIDTH] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_INIT_ADDRESS_WIDTH",
        .ee_format = "initializer for '%s' needs a pointer to hold an address"
    },

    // What: Can't initialize a file-scope object with a run-time value
    // Args: [name]
    [ERR_GEN_INIT_NOT_CONSTANT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_INIT_NOT_CONSTANT",
        .ee_format = "initializer for '%s' is not a constant"
    },

    // What: Can't link two definitions of one symbol
    // Args: [symbol]
    [ERR_LINK_MULTIPLE_DEFINITION] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LINK_MULTIPLE_DEFINITION",
        .ee_format = "multiple definition of '%s'"
    },

    // What: Can't link an object file that can't be read
    // Args: [path]
    [ERR_LINK_OBJECT_UNREADABLE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LINK_OBJECT_UNREADABLE",
        .ee_format = "cannot read object '%s'"
    },

    // What: Can't link a reference to a symbol no object defines
    // Args: [symbol]
    [ERR_LINK_UNDEFINED_SYMBOL] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LINK_UNDEFINED_SYMBOL",
        .ee_format = "undefined symbol '%s'"
    },

    // What: Can't link without a definition of the entry symbol
    // Args: [symbol]
    [ERR_LINK_UNDEFINED_ENTRY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LINK_UNDEFINED_ENTRY",
        .ee_format = "undefined entry symbol '%s'"
    },

    // What: Can't link a relocation type the linker does not know
    // Args: [relocation type]
    [ERR_LINK_UNSUPPORTED_RELOCATION] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LINK_UNSUPPORTED_RELOCATION",
        .ee_format = "unsupported relocation type %u"
    },

    // What: Can't load a file that is not ELF
    // Args: [path]
    [ERR_LOAD_NOT_ELF] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LOAD_NOT_ELF",
        .ee_format = "not an ELF file: '%s'"
    },

    // What: Can't load an ELF file that is not 64-bit little-endian
    // Args: [path]
    [ERR_LOAD_NOT_ELF64_LSB] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LOAD_NOT_ELF64_LSB",
        .ee_format = "not a 64-bit little-endian ELF file: '%s'"
    },

    // What: Can't load an ELF file that is not an executable
    // Args: [path]
    [ERR_LOAD_NOT_EXECUTABLE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LOAD_NOT_EXECUTABLE",
        .ee_format = "not an executable: '%s'"
    },

    // What: Can't load an executable with no loadable segments
    // Args: [path]
    [ERR_LOAD_NO_SEGMENTS] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LOAD_NO_SEGMENTS",
        .ee_format = "no loadable segments in '%s'"
    },

    // What: Can't load a segment that runs past the end of its file
    // Args: [path]
    [ERR_LOAD_SEGMENT_TRUNCATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LOAD_SEGMENT_TRUNCATED",
        .ee_format = "segment runs past the end of '%s'"
    },

    // What: Can't give `.quad` an operand that is not an address
    // Args: [operand]
    [ERR_TXT_QUAD_NOT_ADDRESS] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_QUAD_NOT_ADDRESS",
        .ee_format = "'%s' is not an address a .quad can hold"
    },

    // What: Can't assemble a line whose mnemonic is malformed
    // Args: [line]
    [ERR_TXT_MNEMONIC_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_MNEMONIC_MALFORMED",
        .ee_format = "bad mnemonic in '%s'"
    },

    // What: Can't assemble a mnemonic the assembler does not know
    // Args: [mnemonic length, mnemonic]
    [ERR_TXT_MNEMONIC_UNKNOWN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_MNEMONIC_UNKNOWN",
        .ee_format = "unknown mnemonic '%.*s'"
    },

    // What: Can't give an instruction too many operands
    // Args: [line]
    [ERR_TXT_OPERANDS_TOO_MANY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_OPERANDS_TOO_MANY",
        .ee_format = "too many operands in '%s'"
    },

    // What: Can't give an indirect operand to an instruction that takes none
    // Args: [line]
    [ERR_TXT_OPERAND_NOT_INDIRECT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_OPERAND_NOT_INDIRECT",
        .ee_format = "'%s' takes no indirect operand"
    },

    // What: Can't assemble a malformed operand
    // Args: [operand]
    [ERR_TXT_OPERAND_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_OPERAND_MALFORMED",
        .ee_format = "bad operand '%s'"
    },

    // What: Can't give a branch anything but one operand
    // Args: [line]
    [ERR_TXT_BRANCH_OPERAND_COUNT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_BRANCH_OPERAND_COUNT",
        .ee_format = "'%s' takes one operand"
    },

    // What: Can't branch to anything but a label
    // Args: [line]
    [ERR_TXT_BRANCH_NOT_LABEL] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_BRANCH_NOT_LABEL",
        .ee_format = "'%s' needs a label"
    },

    // What: Can't leave a string without its closing quote
    // Args: [string]
    [ERR_TXT_STRING_UNTERMINATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_STRING_UNTERMINATED",
        .ee_format = "missing closing quote in '%s'"
    },

    // What: Can't use an escape sequence the assembler does not know
    // Args: [character]
    [ERR_TXT_ESCAPE_UNKNOWN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_ESCAPE_UNKNOWN",
        .ee_format = "unknown escape sequence '\\%c'"
    },

    // What: Can't target an architecture other than x86_64
    // Args: [architecture, supported architecture]
    [ERR_CC_ARCH_UNSUPPORTED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_CC_ARCH_UNSUPPORTED",
        .ee_format = "unsupported architecture '%s' (only %s is supported)"
    },

    // What: Can't link without finding the runtime directory
    // Args: none
    [ERR_CC_RUNTIME_NOT_FOUND] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_CC_RUNTIME_NOT_FOUND",
        .ee_format = "cannot locate the runtime directory; pass -B DIR"
    },

    // What: Can't compile to a standard other than C99
    // Args: [standard, supported standard]
    [ERR_CC_STD_UNSUPPORTED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_CC_STD_UNSUPPORTED",
        .ee_format = "unsupported standard '%s' (only %s is supported)"
    },

    // What: Can't pass `-place` anything but SEC@ADDR
    // Args: [placement]
    [ERR_LD_PLACE_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LD_PLACE_MALFORMED",
        .ee_format = "malformed -place (expected SEC@ADDR): '%s'"
    },

    // What: Can't emulate an architecture other than x86_64
    // Args: [architecture, supported architecture]
    [ERR_EMU_ARCH_UNSUPPORTED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_ARCH_UNSUPPORTED",
        .ee_format = "unsupported architecture '%s' (only %s is supported)"
    },

    // What: Can't run an executable that is not for x86_64
    // Args: [path]
    [ERR_EMU_NOT_X86_64] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_NOT_X86_64",
        .ee_format = "'%s' is not an x86_64 executable"
    },

    // What: Can't read unmapped memory
    // Args: [address, instruction address]
    [ERR_EMU_READ_UNMAPPED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_READ_UNMAPPED",
        .ee_format = "read of unmapped memory at 0x%llx from %%rip = 0x%llx"
    },

    // What: Can't write to unmapped memory
    // Args: [address, instruction address]
    [ERR_EMU_WRITE_UNMAPPED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_WRITE_UNMAPPED",
        .ee_format = "write to unmapped memory at 0x%llx from %%rip = 0x%llx"
    },

    // What: Can't pass a syscall a buffer in unmapped memory
    // Args: [address, instruction address]
    [ERR_EMU_SYSCALL_UNMAPPED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_SYSCALL_UNMAPPED",
        .ee_format = "syscall buffer in unmapped memory at 0x%llx from %%rip = 0x%llx"
    },

    // What: Can't make a syscall the emulator does not implement
    // Args: [syscall number, instruction address]
    [ERR_EMU_SYSCALL_UNIMPLEMENTED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_SYSCALL_UNIMPLEMENTED",
        .ee_format = "unimplemented syscall %llu from %%rip = 0x%llx"
    },

    // What: Can't run an instruction that does not decode
    // Args: [instruction address]
    [ERR_EMU_UNDECODABLE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_UNDECODABLE",
        .ee_format = "undecodable instruction at 0x%llx"
    },

    // What: Can't run an opcode the emulator does not implement
    // Args: [opcode map, instruction address]
    [ERR_EMU_OPCODE_UNIMPLEMENTED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_OPCODE_UNIMPLEMENTED",
        .ee_format = "unimplemented %s opcode at 0x%llx"
    },

    // What: Can't divide by zero
    // Args: [instruction address]
    [ERR_EMU_DIVIDE_BY_ZERO] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_DIVIDE_BY_ZERO",
        .ee_format = "divide by zero at 0x%llx"
    }
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

    va_list copy;
    va_copy(copy, ap);
    size_t len = (size_t) vsnprintf(NULL, 0, Err_Table[code].ee_format, copy);
    va_end(copy);

    char *message = malloc(len + 1);
    vsnprintf(message, len + 1, Err_Table[code].ee_format, ap);
    Log_Show(severity, line, "%s [%s]", message, Err_Table[code].ee_name);
    free(message);
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
