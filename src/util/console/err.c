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
    // Args: [path, reason]
    [ERR_FILE_ACCESS] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_FILE_ACCESS",
        .ee_format = "%s: %s"
    },

    // Args: none
    [ERR_PP_COMMENT_UNTERMINATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_COMMENT_UNTERMINATED",
        .ee_format = "unterminated comment"
    },

    // Args: [name length, name]
    [ERR_PP_DIRECTIVE_UNKNOWN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_DIRECTIVE_UNKNOWN",
        .ee_format = "invalid preprocessing directive #%.*s"
    },

    // Args: [directive length, directive]
    [ERR_PP_EXTRA_TOKENS] = {
        .ee_level  = ERR_LEVEL_PEDANTIC,
        .ee_name   = "ERR_PP_EXTRA_TOKENS",
        .ee_format = "extra tokens at end of #%.*s directive"
    },

    // Args: none
    [ERR_PP_INCLUDE_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_INCLUDE_MALFORMED",
        .ee_format = "#include expects \"FILENAME\" or <FILENAME>"
    },

    // Args: [name]
    [ERR_PP_INCLUDE_NOT_FOUND] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_INCLUDE_NOT_FOUND",
        .ee_format = "include file '%s' not found"
    },

    // Args: [limit]
    [ERR_PP_INCLUDE_TOO_DEEP] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_INCLUDE_TOO_DEEP",
        .ee_format = "#include nested more than %d deep"
    },

    // Args: none
    [ERR_PP_MACRO_NAME_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_MACRO_NAME_MISSING",
        .ee_format = "macro names must be identifiers"
    },

    // Args: [name length, name]
    [ERR_PP_MACRO_REDEFINED] = {
        .ee_level  = ERR_LEVEL_PEDANTIC,
        .ee_name   = "ERR_PP_MACRO_REDEFINED",
        .ee_format = "'%.*s' redefined"
    },

    // Args: none
    [ERR_PP_MACRO_PARAMS_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_MACRO_PARAMS_MALFORMED",
        .ee_format = "invalid macro parameter list"
    },

    // Args: [name length, name]
    [ERR_PP_MACRO_PARAM_DUPLICATE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_MACRO_PARAM_DUPLICATE",
        .ee_format = "duplicate macro parameter '%.*s'"
    },

    // Args: [name]
    [ERR_PP_MACRO_UNTERMINATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_MACRO_UNTERMINATED",
        .ee_format = "unterminated argument list invoking macro '%s'"
    },

    // Args: [name, required, given]
    [ERR_PP_MACRO_ARGS_COUNT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_MACRO_ARGS_COUNT",
        .ee_format = "macro '%s' requires %zu arguments, but %zu given"
    },

    // Args: none
    [ERR_PP_VA_ARGS_MISPLACED] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PP_VA_ARGS_MISPLACED",
        .ee_format = "__VA_ARGS__ can only appear in the expansion of a variadic macro"
    },

    // Args: none
    [ERR_PP_STRINGIZE_NOT_PARAM] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_STRINGIZE_NOT_PARAM",
        .ee_format = "'#' is not followed by a macro parameter"
    },

    // Args: none
    [ERR_PP_PASTE_AT_EDGE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_PASTE_AT_EDGE",
        .ee_format = "'##' cannot appear at either end of a macro expansion"
    },

    // Args: [left length, left, right length, right]
    [ERR_PP_PASTE_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_PASTE_INVALID",
        .ee_format = "pasting \"%.*s\" and \"%.*s\" does not give a valid preprocessing token"
    },

    // Args: [directive length, directive]
    [ERR_PP_COND_UNTERMINATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_COND_UNTERMINATED",
        .ee_format = "unterminated #%.*s"
    },

    // Args: [directive length, directive]
    [ERR_PP_COND_WITHOUT_IF] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_COND_WITHOUT_IF",
        .ee_format = "#%.*s without #if"
    },

    // Args: [directive length, directive]
    [ERR_PP_COND_AFTER_ELSE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_COND_AFTER_ELSE",
        .ee_format = "#%.*s after #else"
    },

    // Args: none
    [ERR_PP_DEFINED_NAME_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_DEFINED_NAME_MISSING",
        .ee_format = "operator \"defined\" requires an identifier"
    },

    // Args: none
    [ERR_PP_DEFINED_PAREN_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_DEFINED_PAREN_MISSING",
        .ee_format = "missing ')' after \"defined\""
    },

    // Args: [directive length, directive]
    [ERR_PP_EXPR_EMPTY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_EMPTY",
        .ee_format = "#%.*s with no expression"
    },

    // Args: none
    [ERR_PP_EXPR_VALUE_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_VALUE_MISSING",
        .ee_format = "expected a value in preprocessor expression"
    },

    // Args: [token length, token]
    [ERR_PP_EXPR_TOKEN_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_TOKEN_INVALID",
        .ee_format = "token \"%.*s\" is not valid in preprocessor expressions"
    },

    // Args: [token length, token]
    [ERR_PP_EXPR_OPERATOR_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_OPERATOR_MISSING",
        .ee_format = "missing binary operator before token \"%.*s\""
    },

    // Args: none
    [ERR_PP_EXPR_PAREN_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_PAREN_MISSING",
        .ee_format = "missing ')' in expression"
    },

    // Args: none
    [ERR_PP_EXPR_COLON_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_COLON_MISSING",
        .ee_format = "'?' without following ':'"
    },

    // Args: none
    [ERR_PP_EXPR_DIVISION_BY_ZERO] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_DIVISION_BY_ZERO",
        .ee_format = "division by zero in #if"
    },

    // Args: none
    [ERR_PP_EXPR_FLOAT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_FLOAT",
        .ee_format = "floating constant in preprocessor expression"
    },

    // Args: [suffix]
    [ERR_PP_EXPR_SUFFIX_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_EXPR_SUFFIX_INVALID",
        .ee_format = "invalid suffix \"%s\" on integer constant"
    },

    // Args: none
    [ERR_PP_EXPR_TOO_LARGE] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PP_EXPR_TOO_LARGE",
        .ee_format = "integer constant is too large for its type"
    },

    // Args: none
    [ERR_PP_LINE_NUMBER_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_LINE_NUMBER_MISSING",
        .ee_format = "#line expects a line number"
    },

    // Args: [token length, token]
    [ERR_PP_LINE_NUMBER_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_LINE_NUMBER_INVALID",
        .ee_format = "\"%.*s\" after #line is not a positive integer"
    },

    // Args: none
    [ERR_PP_LINE_OUT_OF_RANGE] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PP_LINE_OUT_OF_RANGE",
        .ee_format = "line number out of range"
    },

    // Args: [token length, token]
    [ERR_PP_LINE_NAME_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_LINE_NAME_INVALID",
        .ee_format = "\"%.*s\" is not a valid filename"
    },

    // Args: none
    [ERR_PP_PRAGMA_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_PRAGMA_MALFORMED",
        .ee_format = "_Pragma takes a parenthesized string literal"
    },

    // Args: none
    [ERR_PP_ONCE_IN_MAIN_FILE] = {
        .ee_level  = ERR_LEVEL_WARNING,
        .ee_name   = "ERR_PP_ONCE_IN_MAIN_FILE",
        .ee_format = "#pragma once in main file"
    },

    // Args: [text]
    [ERR_PP_ERROR_DIRECTIVE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PP_ERROR_DIRECTIVE",
        .ee_format = "#error %s"
    },

    // Args: [text]
    [ERR_PP_WARNING_DIRECTIVE] = {
        .ee_level  = ERR_LEVEL_WARNING,
        .ee_name   = "ERR_PP_WARNING_DIRECTIVE",
        .ee_format = "#warning %s"
    },

    // Args: [character]
    [ERR_LEX_UNEXPECTED_CHAR] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LEX_UNEXPECTED_CHAR",
        .ee_format = "unexpected character '%s'"
    },

    // Args: [parser message]
    [ERR_PAR_SYNTAX] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SYNTAX",
        .ee_format = "%s"
    },

    // Args: [length]
    [ERR_PAR_TEXT_TOO_LONG] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_TEXT_TOO_LONG",
        .ee_format = "preprocessed text of %zu bytes is too long for the lexer"
    },

    // Args: [element size]
    [ERR_PAR_ESCAPE_OUT_OF_RANGE] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_ESCAPE_OUT_OF_RANGE",
        .ee_format = "escape sequence out of range for a %zu-byte character"
    },

    // Args: none
    [ERR_PAR_ESCAPE_HEX_EMPTY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ESCAPE_HEX_EMPTY",
        .ee_format = "\\x used with no following hex digits"
    },

    // Args: none
    [ERR_PAR_ESCAPE_UCN_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ESCAPE_UCN_INCOMPLETE",
        .ee_format = "incomplete universal character name"
    },

    // Args: [character]
    [ERR_PAR_ESCAPE_UNKNOWN] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_ESCAPE_UNKNOWN",
        .ee_format = "unknown escape sequence '\\%c'"
    },

    // Args: none
    [ERR_PAR_DECL_UNNAMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_DECL_UNNAMED",
        .ee_format = "this declaration needs a name"
    },

    // Args: none
    [ERR_PAR_ARRAY_OF_FUNCTIONS] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_OF_FUNCTIONS",
        .ee_format = "an array of functions is not a type"
    },

    // Args: none
    [ERR_PAR_ARRAY_LEN_NOT_CONSTANT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_LEN_NOT_CONSTANT",
        .ee_format = "an array length is not a constant"
    },

    // Args: none
    [ERR_PAR_ARRAY_LEN_NOT_POSITIVE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_LEN_NOT_POSITIVE",
        .ee_format = "an array length must be greater than zero"
    },

    // Args: none
    [ERR_PAR_ARRAY_DECOR_NOT_PARAM] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_DECOR_NOT_PARAM",
        .ee_format = "'static' and qualifiers in an array declarator are only allowed on a parameter"
    },

    // Args: none
    [ERR_PAR_ARRAY_DECOR_NOT_OUTERMOST] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_DECOR_NOT_OUTERMOST",
        .ee_format = "'static' and qualifiers are only allowed on a parameter's outermost array"
    },

    // Args: none
    [ERR_PAR_ARRAY_STATIC_NO_LEN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ARRAY_STATIC_NO_LEN",
        .ee_format = "'static' in an array declarator needs a length"
    },

    // Args: none
    [ERR_PAR_VLA_STAR_NOT_PROTOTYPE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_VLA_STAR_NOT_PROTOTYPE",
        .ee_format = "'[*]' is allowed only in a prototype's parameters"
    },

    // Args: [name]
    [ERR_PAR_ARRAY_ASSUMED_ONE] = {
        .ee_level  = ERR_LEVEL_WARNING,
        .ee_name   = "ERR_PAR_ARRAY_ASSUMED_ONE",
        .ee_format = "array '%s' assumed to have one element"
    },

    // Args: none
    [ERR_PAR_FUNCTION_BAD_RETURN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_FUNCTION_BAD_RETURN",
        .ee_format = "a function cannot return a function or an array"
    },

    // Args: [parameter]
    [ERR_PAR_KNR_NOT_PARAM] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_KNR_NOT_PARAM",
        .ee_format = "'%s' is not a parameter of this function"
    },

    // Args: [parameter]
    [ERR_PAR_KNR_UNDECLARED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_KNR_UNDECLARED",
        .ee_format = "parameter '%s' has no declaration"
    },

    // Args: none
    [ERR_PAR_SPEC_REPEATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SPEC_REPEATED",
        .ee_format = "a type specifier is repeated"
    },

    // Args: none
    [ERR_PAR_SPEC_TWO_TYPES] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SPEC_TWO_TYPES",
        .ee_format = "two or more data types in one declaration"
    },

    // Args: none
    [ERR_PAR_SPEC_MISSING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SPEC_MISSING",
        .ee_format = "a declaration needs a type specifier"
    },

    // Args: none
    [ERR_PAR_SPEC_INVALID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SPEC_INVALID",
        .ee_format = "these type specifiers do not name a type"
    },

    // Args: none
    [ERR_PAR_STORAGE_REPEATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_STORAGE_REPEATED",
        .ee_format = "a declaration takes at most one storage class"
    },

    // Args: none
    [ERR_PAR_STORAGE_NOT_ALLOWED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_STORAGE_NOT_ALLOWED",
        .ee_format = "a storage class or 'inline' is not allowed here"
    },

    // Args: none
    [ERR_PAR_VOID_SIGNED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_VOID_SIGNED",
        .ee_format = "'void' cannot be signed or unsigned"
    },

    // Args: none
    [ERR_PAR_BOOL_SIGNED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BOOL_SIGNED",
        .ee_format = "'_Bool' cannot be signed or unsigned"
    },

    // Args: none
    [ERR_PAR_FLOAT_SIGNED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_FLOAT_SIGNED",
        .ee_format = "a floating type cannot be signed or unsigned"
    },

    // Args: none
    [ERR_PAR_VA_ARG_TYPE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_VA_ARG_TYPE",
        .ee_format = "__builtin_va_arg needs a complete object type that is not an array"
    },

    // Args: none
    [ERR_PAR_BITFIELD_NOT_CONSTANT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BITFIELD_NOT_CONSTANT",
        .ee_format = "a bit-field width is not a constant"
    },

    // Args: none
    [ERR_PAR_BITFIELD_NOT_INTEGER] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BITFIELD_NOT_INTEGER",
        .ee_format = "a bit-field must have an integer type"
    },

    // Args: none
    [ERR_PAR_BITFIELD_NEGATIVE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BITFIELD_NEGATIVE",
        .ee_format = "a bit-field width cannot be negative"
    },

    // Args: none
    [ERR_PAR_BITFIELD_TOO_WIDE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BITFIELD_TOO_WIDE",
        .ee_format = "a bit-field is wider than the type that holds it"
    },

    // Args: none
    [ERR_PAR_BITFIELD_NAMED_ZERO] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BITFIELD_NAMED_ZERO",
        .ee_format = "a bit-field with a name cannot be zero bits wide"
    },

    // Args: none
    [ERR_PAR_MEMBER_UNNAMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_MEMBER_UNNAMED",
        .ee_format = "this member needs a name"
    },

    // Args: [tag]
    [ERR_PAR_TAG_REDEFINED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_TAG_REDEFINED",
        .ee_format = "redefinition of '%s'"
    },

    // Args: [tag]
    [ERR_PAR_TAG_WRONG_KIND] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_TAG_WRONG_KIND",
        .ee_format = "'%s' was declared with a different aggregate keyword"
    },

    // Args: [enumerator]
    [ERR_PAR_ENUM_NOT_CONSTANT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ENUM_NOT_CONSTANT",
        .ee_format = "enumerator '%s' is not a constant"
    },

    // Args: [member]
    [ERR_PAR_DESIG_NOT_AGGREGATE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_DESIG_NOT_AGGREGATE",
        .ee_format = "'.%s' designates a member of something that is not a struct or union"
    },

    // Args: [member]
    [ERR_PAR_DESIG_NO_MEMBER] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_DESIG_NO_MEMBER",
        .ee_format = "no member named '%s' to initialize"
    },

    // Args: none
    [ERR_PAR_DESIG_NOT_ARRAY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_DESIG_NOT_ARRAY",
        .ee_format = "an index designator needs an array"
    },

    // Args: [index]
    [ERR_PAR_DESIG_OUT_OF_RANGE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_DESIG_OUT_OF_RANGE",
        .ee_format = "initializer index %ld is outside the array"
    },

    // Args: [array length]
    [ERR_PAR_INIT_TOO_MANY_ELEMENTS] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_INIT_TOO_MANY_ELEMENTS",
        .ee_format = "too many initializers for an array of %d"
    },

    // Args: [type name]
    [ERR_PAR_INIT_TOO_MANY_MEMBERS] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_INIT_TOO_MANY_MEMBERS",
        .ee_format = "too many initializers for '%s'"
    },

    // Args: none
    [ERR_PAR_INIT_ARRAY_UNBRACED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_INIT_ARRAY_UNBRACED",
        .ee_format = "an array needs a braced initializer"
    },

    // Args: none
    [ERR_PAR_INIT_EMPTY] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_INIT_EMPTY",
        .ee_format = "an empty initializer list has nothing to assign"
    },

    // Args: [element size, character size]
    [ERR_PAR_INIT_STRING_WIDTH] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_INIT_STRING_WIDTH",
        .ee_format = "an array of %d-byte elements cannot take a string of %d-byte characters"
    },

    // Args: [array length]
    [ERR_PAR_INIT_STRING_TOO_LONG] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_PAR_INIT_STRING_TOO_LONG",
        .ee_format = "initializer string is too long for an array of %d"
    },

    // Args: none
    [ERR_PAR_LITERAL_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_LITERAL_INCOMPLETE",
        .ee_format = "a compound literal of an incomplete type has no size"
    },

    // Args: [name]
    [ERR_PAR_OBJECT_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_OBJECT_INCOMPLETE",
        .ee_format = "'%s' has an incomplete type"
    },

    // Args: [name]
    [ERR_PAR_OBJECT_REDEFINED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_OBJECT_REDEFINED",
        .ee_format = "redefinition of '%s'"
    },

    // Args: [name]
    [ERR_PAR_OBJECT_LINKAGE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_OBJECT_LINKAGE",
        .ee_format = "'%s' is declared both static and not"
    },

    // Args: [name]
    [ERR_PAR_EXTERN_INITIALIZED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_EXTERN_INITIALIZED",
        .ee_format = "'%s' is extern inside a block and takes no initializer"
    },

    // Args: none
    [ERR_PAR_SIZEOF_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_SIZEOF_INCOMPLETE",
        .ee_format = "invalid application of 'sizeof' to an incomplete type"
    },

    // Args: none
    [ERR_PAR_TYPEDEF_INITIALIZED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_TYPEDEF_INITIALIZED",
        .ee_format = "a typedef takes no initializer"
    },

    // Args: [name]
    [ERR_PAR_VLA_INITIALIZED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_VLA_INITIALIZED",
        .ee_format = "variable-length array '%s' takes no initializer"
    },

    // Args: [identifier]
    [ERR_PAR_UNDECLARED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_UNDECLARED",
        .ee_format = "use of undeclared identifier '%s'"
    },

    // Args: [name]
    [ERR_PAR_BODY_NOT_FUNCTION] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_BODY_NOT_FUNCTION",
        .ee_format = "'%s' is not a function and takes no body"
    },

    // Args: [name]
    [ERR_PAR_REDECLARED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_REDECLARED",
        .ee_format = "redeclaration of '%s' in the same scope"
    },

    // Args: [name]
    [ERR_PAR_CONFLICTING_TYPES] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_CONFLICTING_TYPES",
        .ee_format = "conflicting types for '%s'"
    },

    // Args: [name]
    [ERR_PAR_FUNCTION_REDEFINED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_FUNCTION_REDEFINED",
        .ee_format = "redefinition of function '%s'"
    },

    // Args: [qualifier]
    [ERR_PAR_ASM_QUALIFIER_DUPLICATE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ASM_QUALIFIER_DUPLICATE",
        .ee_format = "duplicate 'asm' qualifier '%s'"
    },

    // Args: none
    [ERR_PAR_ASM_WIDE_STRING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_PAR_ASM_WIDE_STRING",
        .ee_format = "a wide string is invalid in this context"
    },

    // Args: none
    [ERR_AST_FLEXIBLE_IN_UNION] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_AST_FLEXIBLE_IN_UNION",
        .ee_format = "a union cannot have a flexible array member"
    },

    // Args: [member]
    [ERR_AST_FLEXIBLE_NOT_LAST] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_AST_FLEXIBLE_NOT_LAST",
        .ee_format = "flexible array member '%s' must come last"
    },

    // Args: none
    [ERR_AST_FLEXIBLE_ALONE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_AST_FLEXIBLE_ALONE",
        .ee_format = "a struct needs a member before a flexible array member"
    },

    // Args: [member]
    [ERR_AST_MEMBER_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_AST_MEMBER_INCOMPLETE",
        .ee_format = "member '%s' has an incomplete type"
    },

    // Args: [member]
    [ERR_AST_MEMBER_DUPLICATE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_AST_MEMBER_DUPLICATE",
        .ee_format = "duplicate member '%s'"
    },

    // Args: none
    [ERR_AST_AGGREGATE_UNNAMED] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_AST_AGGREGATE_UNNAMED",
        .ee_format = "an aggregate must declare at least one named member"
    },

    // Args: none
    [ERR_SEM_DIVISION_BY_ZERO] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_DIVISION_BY_ZERO",
        .ee_format = "division by zero in a constant expression"
    },

    // Args: none
    [ERR_SEM_CALL_NOT_FUNCTION] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CALL_NOT_FUNCTION",
        .ee_format = "called object is not a function or function pointer"
    },

    // Args: [callee, count given, count wanted]
    [ERR_SEM_ARGS_TOO_FEW] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ARGS_TOO_FEW",
        .ee_format = "too few arguments to %s: got %d, expected at least %d"
    },

    // Args: [callee, count given, count wanted]
    [ERR_SEM_ARGS_WRONG_COUNT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ARGS_WRONG_COUNT",
        .ee_format = "wrong number of arguments to %s: got %d, expected %d"
    },

    // Args: none
    [ERR_SEM_ADD_POINTERS] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ADD_POINTERS",
        .ee_format = "cannot add two pointers"
    },

    // Args: none
    [ERR_SEM_SUB_POINTER_FROM_INT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_SUB_POINTER_FROM_INT",
        .ee_format = "cannot subtract a pointer from an integer"
    },

    // Args: [label]
    [ERR_SEM_GOTO_UNDEFINED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_GOTO_UNDEFINED",
        .ee_format = "goto names an undefined label '%s'"
    },

    // Args: [label]
    [ERR_SEM_GOTO_INTO_VM_SCOPE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_GOTO_INTO_VM_SCOPE",
        .ee_format = "goto '%s' jumps into the scope of a variably modified name"
    },

    // Args: none
    [ERR_SEM_CASE_DUPLICATE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CASE_DUPLICATE",
        .ee_format = "duplicate case in switch"
    },

    // Args: none
    [ERR_SEM_CASE_INTO_VM_SCOPE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CASE_INTO_VM_SCOPE",
        .ee_format = "switch jumps into the scope of a variably modified name"
    },

    // Args: none
    [ERR_SEM_CASE_NOT_CONSTANT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CASE_NOT_CONSTANT",
        .ee_format = "case label is not a constant"
    },

    // Args: none
    [ERR_SEM_ADDRESS_NOT_LVALUE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ADDRESS_NOT_LVALUE",
        .ee_format = "cannot take the address of this expression"
    },

    // Args: none
    [ERR_SEM_ADDRESS_BITFIELD] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ADDRESS_BITFIELD",
        .ee_format = "cannot take the address of a bit-field"
    },

    // Args: none
    [ERR_SEM_DEREF_NOT_POINTER] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_DEREF_NOT_POINTER",
        .ee_format = "indirection requires a pointer operand"
    },

    // Args: none
    [ERR_SEM_DEREF_VOID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_DEREF_VOID",
        .ee_format = "cannot dereference a pointer to void"
    },

    // Args: none
    [ERR_SEM_DEREF_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_DEREF_INCOMPLETE",
        .ee_format = "cannot dereference a pointer to an incomplete type"
    },

    // Args: none
    [ERR_SEM_SIZEOF_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_SIZEOF_INCOMPLETE",
        .ee_format = "invalid application of 'sizeof' to an incomplete type"
    },

    // Args: none
    [ERR_SEM_ARRAY_LEN_NOT_INTEGER] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ARRAY_LEN_NOT_INTEGER",
        .ee_format = "an array length is not an integer"
    },

    // Args: [member]
    [ERR_SEM_MEMBER_NOT_AGGREGATE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_MEMBER_NOT_AGGREGATE",
        .ee_format = "request for member '%s' in something that is not a struct or union"
    },

    // Args: [type name]
    [ERR_SEM_MEMBER_INCOMPLETE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_MEMBER_INCOMPLETE",
        .ee_format = "'%s' is an incomplete type"
    },

    // Args: [member, type name]
    [ERR_SEM_MEMBER_UNKNOWN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_MEMBER_UNKNOWN",
        .ee_format = "no member named '%s' in '%s'"
    },

    // Args: none
    [ERR_SEM_NOT_ASSIGNABLE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_NOT_ASSIGNABLE",
        .ee_format = "expression is not assignable"
    },

    // Args: none
    [ERR_SEM_ASSIGN_ARRAY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ASSIGN_ARRAY",
        .ee_format = "cannot assign to an array"
    },

    // Args: none
    [ERR_SEM_ASSIGN_CONST] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ASSIGN_CONST",
        .ee_format = "cannot store to a read-only object or an array"
    },

    // Args: [context]
    [ERR_SEM_ASSIGN_INCOMPATIBLE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_ASSIGN_INCOMPATIBLE",
        .ee_format = "incompatible types in %s"
    },

    // Args: [context]
    [ERR_SEM_ASSIGN_DISCARDS_QUALIFIER] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_SEM_ASSIGN_DISCARDS_QUALIFIER",
        .ee_format = "%s discards a qualifier of the pointed-to type"
    },

    // Args: none
    [ERR_SEM_VOID_VALUE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_VOID_VALUE",
        .ee_format = "a void value cannot be used"
    },

    // Args: none
    [ERR_SEM_COND_MISMATCH] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_COND_MISMATCH",
        .ee_format = "the operands of '?:' have incompatible types"
    },

    // Args: none
    [ERR_SEM_RETURN_VALUE_IN_VOID] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_RETURN_VALUE_IN_VOID",
        .ee_format = "a function returning void cannot return a value"
    },

    // Args: none
    [ERR_SEM_RETURN_NO_VALUE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_RETURN_NO_VALUE",
        .ee_format = "a function returning a value needs one in every return"
    },

    // Args: none
    [ERR_SEM_VA_START_FIXED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_VA_START_FIXED",
        .ee_format = "__builtin_va_start outside a variadic function"
    },

    // Args: none
    [ERR_SEM_POINTER_FLOATING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_POINTER_FLOATING",
        .ee_format = "pointer arithmetic needs an integer, not a floating operand"
    },

    // Args: none
    [ERR_SEM_OPERAND_FLOATING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_OPERAND_FLOATING",
        .ee_format = "this operator needs integer operands, not floating ones"
    },

    // Args: none
    [ERR_SEM_CAST_FLOATING] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CAST_FLOATING",
        .ee_format = "cannot convert between a floating type and a non-arithmetic one"
    },

    // Args: none
    [ERR_SEM_OPERAND_NOT_ARITHMETIC] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_OPERAND_NOT_ARITHMETIC",
        .ee_format = "this operator takes only arithmetic operands"
    },

    // Args: none
    [ERR_SEM_OPERAND_NOT_INTEGER] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_OPERAND_NOT_INTEGER",
        .ee_format = "this operator takes only integer operands"
    },

    // Args: none
    [ERR_SEM_OPERAND_NOT_SCALAR] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_OPERAND_NOT_SCALAR",
        .ee_format = "a value tested for truth must be a number or a pointer"
    },

    // Args: none
    [ERR_SEM_CAST_NOT_SCALAR] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_SEM_CAST_NOT_SCALAR",
        .ee_format = "a cast converts only a number or a pointer, to one or to void"
    },

    // Args: none
    [ERR_GEN_NOT_LVALUE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_NOT_LVALUE",
        .ee_format = "not an lvalue"
    },

    // Args: [operator kind]
    [ERR_GEN_UNEXPECTED_OPASSIGN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_UNEXPECTED_OPASSIGN",
        .ee_format = "unexpected compound assignment %d"
    },

    // Args: [node kind]
    [ERR_GEN_UNEXPECTED_EXPR] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_UNEXPECTED_EXPR",
        .ee_format = "unexpected node kind %d"
    },

    // Args: [node kind]
    [ERR_GEN_UNEXPECTED_STMT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_UNEXPECTED_STMT",
        .ee_format = "unexpected statement kind %d"
    },

    // Args: none
    [ERR_GEN_BREAK_OUTSIDE_LOOP] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_BREAK_OUTSIDE_LOOP",
        .ee_format = "break outside a loop"
    },

    // Args: none
    [ERR_GEN_CONTINUE_OUTSIDE_LOOP] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_CONTINUE_OUTSIDE_LOOP",
        .ee_format = "continue outside a loop"
    },

    // Args: [name]
    [ERR_GEN_INIT_TOO_LARGE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_INIT_TOO_LARGE",
        .ee_format = "initializer for '%s' is larger than it is"
    },

    // Args: [name]
    [ERR_GEN_INIT_ADDRESS_WIDTH] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_INIT_ADDRESS_WIDTH",
        .ee_format = "initializer for '%s' needs a pointer to hold an address"
    },

    // Args: [name]
    [ERR_GEN_INIT_NOT_CONSTANT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_GEN_INIT_NOT_CONSTANT",
        .ee_format = "initializer for '%s' is not a constant"
    },

    // Args: [symbol]
    [ERR_LINK_MULTIPLE_DEFINITION] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LINK_MULTIPLE_DEFINITION",
        .ee_format = "multiple definition of '%s'"
    },

    // Args: [path]
    [ERR_LINK_OBJECT_UNREADABLE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LINK_OBJECT_UNREADABLE",
        .ee_format = "cannot read object '%s'"
    },

    // Args: [symbol]
    [ERR_LINK_UNDEFINED_SYMBOL] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LINK_UNDEFINED_SYMBOL",
        .ee_format = "undefined symbol '%s'"
    },

    // Args: [symbol]
    [ERR_LINK_UNDEFINED_ENTRY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LINK_UNDEFINED_ENTRY",
        .ee_format = "undefined entry symbol '%s'"
    },

    // Args: [relocation type]
    [ERR_LINK_UNSUPPORTED_RELOCATION] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LINK_UNSUPPORTED_RELOCATION",
        .ee_format = "unsupported relocation type %u"
    },

    // Args: [path]
    [ERR_LOAD_NOT_ELF] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LOAD_NOT_ELF",
        .ee_format = "not an ELF file: '%s'"
    },

    // Args: [path]
    [ERR_LOAD_NOT_ELF64_LSB] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LOAD_NOT_ELF64_LSB",
        .ee_format = "not a 64-bit little-endian ELF file: '%s'"
    },

    // Args: [path]
    [ERR_LOAD_NOT_EXECUTABLE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LOAD_NOT_EXECUTABLE",
        .ee_format = "not an executable: '%s'"
    },

    // Args: [path]
    [ERR_LOAD_NO_SEGMENTS] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LOAD_NO_SEGMENTS",
        .ee_format = "no loadable segments in '%s'"
    },

    // Args: [path]
    [ERR_LOAD_SEGMENT_TRUNCATED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LOAD_SEGMENT_TRUNCATED",
        .ee_format = "segment runs past the end of '%s'"
    },

    // Args: [operand]
    [ERR_TXT_QUAD_NOT_ADDRESS] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_QUAD_NOT_ADDRESS",
        .ee_format = "'%s' is not an address a .quad can hold"
    },

    // Args: [line]
    [ERR_TXT_MNEMONIC_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_MNEMONIC_MALFORMED",
        .ee_format = "bad mnemonic in '%s'"
    },

    // Args: [mnemonic length, mnemonic]
    [ERR_TXT_MNEMONIC_UNKNOWN] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_MNEMONIC_UNKNOWN",
        .ee_format = "unknown mnemonic '%.*s'"
    },

    // Args: [line]
    [ERR_TXT_OPERANDS_TOO_MANY] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_OPERANDS_TOO_MANY",
        .ee_format = "too many operands in '%s'"
    },

    // Args: [line]
    [ERR_TXT_OPERAND_NOT_INDIRECT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_OPERAND_NOT_INDIRECT",
        .ee_format = "'%s' takes no indirect operand"
    },

    // Args: [operand]
    [ERR_TXT_OPERAND_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_OPERAND_MALFORMED",
        .ee_format = "bad operand '%s'"
    },

    // Args: [line]
    [ERR_TXT_BRANCH_OPERAND_COUNT] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_BRANCH_OPERAND_COUNT",
        .ee_format = "'%s' takes one operand"
    },

    // Args: [line]
    [ERR_TXT_BRANCH_NOT_LABEL] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_TXT_BRANCH_NOT_LABEL",
        .ee_format = "'%s' needs a label"
    },

    // Args: [string]
    [ERR_TXT_STRING_UNTERMINATED] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_TXT_STRING_UNTERMINATED",
        .ee_format = "missing closing quote in '%s'"
    },

    // Args: [character]
    [ERR_TXT_ESCAPE_UNKNOWN] = {
        .ee_level  = ERR_LEVEL_ERROR,
        .ee_name   = "ERR_TXT_ESCAPE_UNKNOWN",
        .ee_format = "unknown escape sequence '\\%c'"
    },

    // Args: [architecture, supported architecture]
    [ERR_CC_ARCH_UNSUPPORTED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_CC_ARCH_UNSUPPORTED",
        .ee_format = "unsupported architecture '%s' (only %s is supported)"
    },

    // Args: none
    [ERR_CC_RUNTIME_NOT_FOUND] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_CC_RUNTIME_NOT_FOUND",
        .ee_format = "cannot locate the runtime directory; pass -B DIR"
    },

    // Args: [standard, supported standard]
    [ERR_CC_STD_UNSUPPORTED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_CC_STD_UNSUPPORTED",
        .ee_format = "unsupported standard '%s' (only %s is supported)"
    },

    // Args: [placement]
    [ERR_LD_PLACE_MALFORMED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_LD_PLACE_MALFORMED",
        .ee_format = "malformed -place (expected SEC@ADDR): '%s'"
    },

    // Args: [architecture, supported architecture]
    [ERR_EMU_ARCH_UNSUPPORTED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_ARCH_UNSUPPORTED",
        .ee_format = "unsupported architecture '%s' (only %s is supported)"
    },

    // Args: [path]
    [ERR_EMU_NOT_X86_64] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_NOT_X86_64",
        .ee_format = "'%s' is not an x86_64 executable"
    },

    // Args: [address, instruction address]
    [ERR_EMU_READ_UNMAPPED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_READ_UNMAPPED",
        .ee_format = "read of unmapped memory at 0x%llx from %%rip = 0x%llx"
    },

    // Args: [address, instruction address]
    [ERR_EMU_WRITE_UNMAPPED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_WRITE_UNMAPPED",
        .ee_format = "write to unmapped memory at 0x%llx from %%rip = 0x%llx"
    },

    // Args: [address, instruction address]
    [ERR_EMU_SYSCALL_UNMAPPED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_SYSCALL_UNMAPPED",
        .ee_format = "syscall buffer in unmapped memory at 0x%llx from %%rip = 0x%llx"
    },

    // Args: [syscall number, instruction address]
    [ERR_EMU_SYSCALL_UNIMPLEMENTED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_SYSCALL_UNIMPLEMENTED",
        .ee_format = "unimplemented syscall %llu from %%rip = 0x%llx"
    },

    // Args: [instruction address]
    [ERR_EMU_UNDECODABLE] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_UNDECODABLE",
        .ee_format = "undecodable instruction at 0x%llx"
    },

    // Args: [opcode map, instruction address]
    [ERR_EMU_OPCODE_UNIMPLEMENTED] = {
        .ee_level  = ERR_LEVEL_FATAL,
        .ee_name   = "ERR_EMU_OPCODE_UNIMPLEMENTED",
        .ee_format = "unimplemented %s opcode at 0x%llx"
    },

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
