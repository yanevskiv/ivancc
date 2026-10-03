/*
 * C header file for user-facing diagnostics.
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

    ERR_PP_FILE_NOT_READABLE,              // can't read a source file or a header
    ERR_PP_COMMENT_NOT_TERMINATED,         // can't leave a comment open at the end of the file
    ERR_PP_DIRECTIVE_NOT_KNOWN,            // can't use a directive the preprocessor does not know
    ERR_PP_EXTRA_TOKENS,                   // shouldn't put tokens after a directive that takes none
    ERR_PP_INCLUDE_MALFORMED,              // can't #include anything but "FILE" or <FILE>
    ERR_PP_INCLUDE_NOT_TERMINATED,         // can't leave the `<` of an #include without its `>`
    ERR_PP_INCLUDE_NOT_FOUND,              // can't #include a file that is not on the search path
    ERR_PP_INCLUDE_TOO_DEEP,               // can't nest #include past the limit
    ERR_PP_MACRO_NAME_MISSING,             // can't name a macro with anything but an identifier
    ERR_PP_MACRO_REDEFINED,                // shouldn't redefine a macro with a different body
    ERR_PP_MACRO_PARAMS_NOT_TERMINATED,    // can't leave a macro parameter list without its `)`
    ERR_PP_MACRO_PARAM_NOT_NAME,           // can't give a macro a parameter that is not a name
    ERR_PP_MACRO_PARAM_VA_ARGS,            // can't name a macro parameter `__VA_ARGS__`
    ERR_PP_MACRO_PARAMS_MALFORMED,         // can't follow a macro parameter with anything but `,` or `)`
    ERR_PP_MACRO_PARAM_DUPLICATE,          // can't give a macro two parameters with the same name
    ERR_PP_MACRO_NOT_TERMINATED,           // can't leave a macro call's argument list open
    ERR_PP_MACRO_ARGS_COUNT,               // can't call a macro with the wrong number of arguments
    ERR_PP_VA_ARGS_NOT_VARIADIC,           // can't use `__VA_ARGS__` in a macro that is not variadic
    ERR_PP_STRINGIZE_NOT_PARAM,            // can't apply `#` to a name that is not a parameter
    ERR_PP_PASTE_AT_EDGE,                  // can't start or end a replacement list with `##`
    ERR_PP_PASTE_NOT_VALID,                // can't paste two tokens that do not form one token
    ERR_PP_COND_NOT_TERMINATED,            // can't leave an #if without its #endif
    ERR_PP_COND_WITHOUT_IF,                // can't have an #endif without an #if
    ERR_PP_ELSE_WITHOUT_IF,                // can't have an #elif or #else without an #if
    ERR_PP_COND_AFTER_ELSE,                // can't have an #elif or #else after #else
    ERR_PP_DEFINED_NAME_MISSING,           // can't apply `defined` to anything but a name
    ERR_PP_DEFINED_PAREN_MISSING,          // can't leave `defined(` without its `)`
    ERR_PP_EXPR_EMPTY,                     // can't have an #if or #elif with no expression
    ERR_PP_EXPR_VALUE_MISSING,             // can't leave out a value in #if
    ERR_PP_EXPR_TOKEN_NOT_VALID,           // can't use a token in #if that has no value there
    ERR_PP_EXPR_OPERATOR_MISSING,          // can't put two values side by side in #if
    ERR_PP_EXPR_PAREN_MISSING,             // can't leave a parenthesis in #if open
    ERR_PP_EXPR_COLON_MISSING,             // can't have a `?` without its `:` in #if
    ERR_PP_EXPR_DIVISION_BY_ZERO,          // can't divide by zero in #if
    ERR_PP_EXPR_FLOAT,                     // can't use a floating constant in #if
    ERR_PP_EXPR_SUFFIX_NOT_VALID,          // can't give an integer in #if a suffix no integer takes
    ERR_PP_EXPR_TOO_LARGE,                 // can't use an integer in #if too large for any type
    ERR_PP_LINE_NUMBER_MISSING,            // can't have a #line without a line number
    ERR_PP_LINE_NUMBER_NOT_VALID,          // can't give #line a line number that is not a number
    ERR_PP_LINE_OUT_OF_RANGE,              // can't #line to line 0 or past 2147483647
    ERR_PP_LINE_NAME_NOT_VALID,            // can't give #line a file name that is not a string literal
    ERR_PP_PRAGMA_MALFORMED,               // can't give _Pragma anything but a string literal
    ERR_PP_ONCE_IN_MAIN_FILE,              // shouldn't use #pragma once in the file being compiled
    ERR_PP_ERROR_DIRECTIVE,                // can't compile past an #error
    ERR_PP_WARNING_DIRECTIVE,              // shouldn't reach a #warning

    ERR_LEX_CHAR_NOT_EXPECTED,             // can't use a character that starts no token

    ERR_PAR_SYNTAX,                        // can't write what C's grammar does not allow
    ERR_PAR_TEXT_TOO_LONG,                 // can't lex more than 2 GiB of preprocessed text
    ERR_PAR_ESCAPE_OUT_OF_RANGE,           // can't have an escape too large for its character type
    ERR_PAR_ESCAPE_HEX_EMPTY,              // can't have a `\x` escape without hex digits
    ERR_PAR_ESCAPE_UCN_NOT_COMPLETE,       // can't have a `\u` or `\U` escape with too few hex digits
    ERR_PAR_ESCAPE_NOT_KNOWN,              // can't use an escape sequence C does not define
    ERR_PAR_DECL_NOT_NAMED,                // can't have a declaration whose declarator names nothing
    ERR_PAR_ARRAY_OF_FUNCTIONS,            // can't have an array of functions
    ERR_PAR_ARRAY_LEN_NOT_CONSTANT,        // can't give an array a run-time length where it needs a constant
    ERR_PAR_ARRAY_LEN_NOT_POSITIVE,        // can't have an array of zero or negative length
    ERR_PAR_ARRAY_DECOR_NOT_PARAM,         // can't put `static` or a qualifier on a non-parameter array
    ERR_PAR_ARRAY_DECOR_NOT_OUTERMOST,     // can't put `static` or a qualifier on a parameter's inner array
    ERR_PAR_ARRAY_STATIC_NO_LEN,           // can't write `[static]` without a length
    ERR_PAR_VLA_STAR_NOT_PROTOTYPE,        // can't use `[*]` outside a prototype's parameters
    ERR_PAR_VLA_OUTSIDE_FUNCTION,          // can't size a variable-length array outside a function
    ERR_PAR_VLA_STATIC,                    // can't give a `static` local a variable-length array type
    ERR_PAR_ARRAY_ASSUMED_ONE,             // shouldn't leave a tentative array without a length
    ERR_PAR_FUNCTION_BAD_RETURN,           // can't have a function return a function or an array
    ERR_PAR_KNR_NOT_PARAM,                 // can't declare a name an old-style parameter list does not have
    ERR_PAR_KNR_NOT_DECLARED,              // can't leave an old-style parameter undeclared
    ERR_PAR_SPEC_REPEATED,                 // can't repeat a type specifier
    ERR_PAR_SPEC_TWO_TYPES,                // can't name two types in one declaration
    ERR_PAR_SPEC_MISSING,                  // can't declare without a type specifier
    ERR_PAR_SPEC_NOT_VALID,                // can't combine type specifiers that name no type
    ERR_PAR_STORAGE_REPEATED,              // can't give one declaration two storage classes
    ERR_PAR_STORAGE_NOT_ALLOWED,           // can't put a storage class or `inline` where it is not allowed
    ERR_PAR_VOID_SIGNED,                   // can't have a signed or unsigned `void`
    ERR_PAR_BOOL_SIGNED,                   // can't have a signed or unsigned `_Bool`
    ERR_PAR_FLOAT_SIGNED,                  // can't have a signed or unsigned floating type
    ERR_PAR_VA_ARG_TYPE,                   // can't take an incomplete type or an array from `__builtin_va_arg`
    ERR_PAR_BITFIELD_NOT_CONSTANT,         // can't have a bit-field width known only at run time
    ERR_PAR_BITFIELD_NOT_INTEGER,          // can't have a bit-field of a type that is not an integer
    ERR_PAR_BITFIELD_NEGATIVE,             // can't have a bit-field of negative width
    ERR_PAR_BITFIELD_TOO_WIDE,             // can't have a bit-field wider than its type
    ERR_PAR_BITFIELD_NAMED_ZERO,           // can't have a named bit-field of zero width
    ERR_PAR_MEMBER_NOT_NAMED,              // can't have a member whose declarator names nothing
    ERR_PAR_TAG_REDEFINED,                 // can't define one tag twice in a scope
    ERR_PAR_TAG_WRONG_KIND,                // can't use a tag with a different aggregate keyword
    ERR_PAR_ENUM_NOT_CONSTANT,             // can't give an enumerator a value known only at run time
    ERR_PAR_ENUM_REDECLARED,               // can't name an enumerator with a name its scope already has
    ERR_PAR_DESIG_NOT_AGGREGATE,           // can't designate a member of something that is not a struct or union
    ERR_PAR_DESIG_NO_MEMBER,               // can't designate a member the struct does not have
    ERR_PAR_DESIG_NOT_ARRAY,               // can't designate an index of something that is not an array
    ERR_PAR_DESIG_OUT_OF_RANGE,            // can't designate an index past the end of the array
    ERR_PAR_DESIG_NOT_CONSTANT,            // can't designate an index known only at run time
    ERR_PAR_INIT_TOO_MANY_ELEMENTS,        // can't give an array more initializers than elements
    ERR_PAR_INIT_TOO_MANY_MEMBERS,         // can't give a struct more initializers than members
    ERR_PAR_INIT_ARRAY_NOT_BRACED,         // can't initialize an array without braces
    ERR_PAR_INIT_EMPTY,                    // can't initialize with empty braces
    ERR_PAR_INIT_STRING_WIDTH,             // can't initialize an array with a string of another width
    ERR_PAR_INIT_STRING_TOO_LONG,          // can't initialize an array with a longer string literal
    ERR_PAR_LITERAL_NOT_COMPLETE,          // can't have a compound literal of an incomplete type
    ERR_PAR_OBJECT_NOT_COMPLETE,           // can't define an object of an incomplete type
    ERR_PAR_OBJECT_REDEFINED,              // can't initialize one file-scope object twice
    ERR_PAR_OBJECT_LINKAGE,                // can't declare an object `static` and then without it
    ERR_PAR_EXTERN_INITIALIZED,            // can't initialize a block-scope `extern`
    ERR_PAR_SIZEOF_NOT_COMPLETE,           // can't take the `sizeof` of an incomplete type
    ERR_PAR_OFFSETOF_NOT_AGGREGATE,        // can't take the offset of a member of something that is not a struct or union
    ERR_PAR_OFFSETOF_NOT_COMPLETE,         // can't take the offset of a member of an incomplete struct or union
    ERR_PAR_OFFSETOF_NO_MEMBER,            // can't take the offset of a member the struct does not have
    ERR_PAR_OFFSETOF_BITFIELD,             // can't take the offset of a bit-field
    ERR_PAR_OFFSETOF_THROUGH_POINTER,      // can't take the offset of an element a pointer points to
    ERR_PAR_OFFSETOF_NOT_ARRAY,            // can't take the offset of an index into something that is not an array
    ERR_PAR_TYPEDEF_INITIALIZED,           // can't initialize a typedef
    ERR_PAR_VLA_INITIALIZED,               // can't initialize a variable-length array
    ERR_PAR_NAME_NOT_DECLARED,             // can't use a name that was never declared
    ERR_PAR_BODY_NOT_FUNCTION,             // can't give an object a function body
    ERR_PAR_REDECLARED,                    // can't declare one name twice in a scope
    ERR_PAR_CONFLICTING_TYPES,             // can't declare one file-scope object with two different types
    ERR_PAR_LOCAL_CONFLICTING_TYPES,       // can't declare one block-scope `extern` or typedef with two different types
    ERR_PAR_FUNCTION_CONFLICTING_TYPES,    // can't declare one function with two different types
    ERR_PAR_FUNCTION_AS_OBJECT,            // can't declare a function's name again as an object
    ERR_PAR_OBJECT_AS_FUNCTION,            // can't declare an object's name again as a function
    ERR_PAR_FUNCTION_REDEFINED,            // can't give one function two bodies
    ERR_PAR_ASM_QUALIFIER_DUPLICATE,       // can't repeat an asm qualifier
    ERR_PAR_ASM_WIDE_STRING,               // can't write an asm template as a wide string

    ERR_AST_FLEXIBLE_IN_UNION,             // can't have a flexible array member in a union
    ERR_AST_FLEXIBLE_NOT_LAST,             // can't have a member after a flexible array member
    ERR_AST_FLEXIBLE_ALONE,                // can't have a flexible array member as a struct's only member
    ERR_AST_MEMBER_NOT_COMPLETE,           // can't have a member of an incomplete type
    ERR_AST_MEMBER_DUPLICATE,              // can't have two members with the same name
    ERR_AST_AGGREGATE_NOT_NAMED,           // can't have a struct or union with no named member

    ERR_SEM_DIVISION_BY_ZERO,              // can't divide by zero in a constant expression
    ERR_SEM_CALL_NOT_FUNCTION,             // can't call something that is not a function or a function pointer
    ERR_SEM_ARGS_TOO_FEW,                  // can't call a variadic function with too few named arguments
    ERR_SEM_ARGS_WRONG_COUNT,              // can't call a function with the wrong number of arguments
    ERR_SEM_ADD_POINTERS,                  // can't add two pointers
    ERR_SEM_SUB_POINTER_FROM_INT,          // can't subtract a pointer from an integer
    ERR_SEM_GOTO_NOT_DEFINED,              // can't goto a label that is never defined
    ERR_SEM_GOTO_INTO_VM_SCOPE,            // can't goto into the scope of a variable-length array
    ERR_SEM_CASE_DUPLICATE,                // can't have two case labels with the same value
    ERR_SEM_CASE_INTO_VM_SCOPE,            // can't jump from a switch into the scope of a variable-length array
    ERR_SEM_CASE_NOT_CONSTANT,             // can't have a case label known only at run time
    ERR_SEM_ADDRESS_NOT_LVALUE,            // can't take the address of a value that is not an lvalue
    ERR_SEM_ADDRESS_BITFIELD,              // can't take the address of a bit-field
    ERR_SEM_DEREF_NOT_POINTER,             // can't dereference something that is not a pointer
    ERR_SEM_DEREF_VOID,                    // can't dereference a `void *` for its value
    ERR_SEM_DEREF_NOT_COMPLETE,            // can't dereference a pointer to an incomplete type
    ERR_SEM_SIZEOF_NOT_COMPLETE,           // can't take the `sizeof` of an expression of incomplete type
    ERR_SEM_ARRAY_LEN_NOT_INTEGER,         // can't give an array a length that is not an integer
    ERR_SEM_MEMBER_NOT_AGGREGATE,          // can't access a member of something that is not a struct or union
    ERR_SEM_MEMBER_NOT_COMPLETE,           // can't access a member of an incomplete type
    ERR_SEM_MEMBER_NOT_KNOWN,              // can't access a member the struct or union does not have
    ERR_SEM_NOT_ASSIGNABLE,                // can't assign with `=` to a value that is not an lvalue
    ERR_SEM_OPASSIGN_NOT_ASSIGNABLE,       // can't assign with `op=` to a value that is not an lvalue
    ERR_SEM_INCDEC_NOT_ASSIGNABLE,         // can't increment or decrement a value that is not an lvalue
    ERR_SEM_ASSIGN_ARRAY,                  // can't assign to an array
    ERR_SEM_ASSIGN_CONST,                  // can't assign with `=` to a `const` object
    ERR_SEM_OPASSIGN_CONST,                // can't assign with `op=` to a `const` object
    ERR_SEM_INCDEC_CONST,                  // can't increment or decrement a `const` object
    ERR_SEM_ASSIGN_NOT_COMPATIBLE,         // can't convert between a struct or union and another type
    ERR_SEM_ASSIGN_NOT_POINTER,            // can't convert between a pointer and a value that is not one
    ERR_SEM_ASSIGN_POINTEE_NOT_COMPATIBLE, // can't convert between pointers to incompatible types
    ERR_SEM_ASSIGN_DISCARDS_QUALIFIER,     // can't drop a qualifier of the pointed-to type in a conversion
    ERR_SEM_VOID_VALUE,                    // can't use a `void` value
    ERR_SEM_COND_MISMATCH,                 // can't choose between a struct or union and another type in a conditional
    ERR_SEM_COND_VOID_MISMATCH,            // can't choose between `void` and a value in a conditional
    ERR_SEM_COND_NOT_POINTER,              // can't choose between a pointer and a value that is not one in a conditional
    ERR_SEM_COND_POINTEE_NOT_COMPATIBLE,   // can't choose between pointers to incompatible types in a conditional
    ERR_SEM_RETURN_VALUE_IN_VOID,          // can't return a value from a `void` function
    ERR_SEM_RETURN_NO_VALUE,               // can't return nothing from a function that returns a value
    ERR_SEM_VA_START_FIXED,                // can't use `__builtin_va_start` in a function that is not variadic
    ERR_SEM_POINTER_FLOATING,              // can't add a floating value to a pointer or subtract one from it
    ERR_SEM_POINTER_OFFSET_NOT_INTEGER,    // can't add to a pointer or subtract from it what is not an integer
    ERR_SEM_OPERAND_FLOATING,              // can't apply an integer operator to a floating operand
    ERR_SEM_CAST_FLOATING,                 // can't cast between a floating type and a pointer
    ERR_SEM_OPERAND_NOT_ARITHMETIC,        // can't apply an arithmetic operator to what is not a number
    ERR_SEM_OPASSIGN_NOT_ARITHMETIC,       // can't apply an arithmetic `op=` to what is not a number
    ERR_SEM_INCDEC_NOT_ARITHMETIC,         // can't increment or decrement what is not a number or a pointer
    ERR_SEM_OPERAND_NOT_INTEGER,           // can't apply an integer operator to what is not an integer
    ERR_SEM_OPERAND_NOT_SCALAR,            // can't test for truth what is not a number or a pointer
    ERR_SEM_CAST_NOT_SCALAR,               // can't cast anything but a number or pointer to one or to `void`

    ERR_GEN_NOT_LVALUE,                    // can't take the address of a value that is not an lvalue
    ERR_GEN_RESULT_NOT_LVALUE,             // can't take the address of an assignment, comma or conditional of scalar type
    ERR_GEN_OPASSIGN_NOT_EXPECTED,         // can't generate code for a compound assignment gen does not know
    ERR_GEN_EXPR_NOT_EXPECTED,             // can't generate code for an expression node gen does not know
    ERR_GEN_STMT_NOT_EXPECTED,             // can't generate code for a statement node gen does not know
    ERR_GEN_BREAK_OUTSIDE_LOOP,            // can't break outside a loop or a switch
    ERR_GEN_CONTINUE_OUTSIDE_LOOP,         // can't continue outside a loop
    ERR_GEN_INIT_TOO_LARGE,                // can't initialize past the end of an object
    ERR_GEN_INIT_ADDRESS_WIDTH,            // can't initialize an object narrower than a pointer with an address
    ERR_GEN_INIT_NOT_CONSTANT,             // can't initialize a file-scope integer or pointer with a run-time value
    ERR_GEN_INIT_FLOAT_NOT_CONSTANT,       // can't initialize a file-scope floating object with a run-time value

    ERR_LINK_MULTIPLE_DEFINITION,          // can't link two definitions of one symbol
    ERR_LINK_OBJECT_NOT_READABLE,          // can't link a file that is no archive and no object the linker reads
    ERR_LINK_SYMBOL_NOT_DEFINED,           // can't link a reference to a symbol no object defines
    ERR_LINK_ENTRY_NOT_DEFINED,            // can't link without a definition of the entry symbol
    ERR_LINK_RELOCATION_NOT_SUPPORTED,     // can't link a relocation type the linker does not know
    ERR_LINK_INPUT_NOT_READABLE,           // can't link a file that can't be read
    ERR_LINK_MEMBER_TRUNCATED,             // can't link an archive with a member that runs past its end
    ERR_LINK_HEADER_MALFORMED,             // can't link an archive with a malformed member header
    ERR_LINK_NAME_NOT_FOUND,               // can't link an archive naming a member outside its long-name table
    ERR_LINK_MEMBER_NOT_READABLE,          // can't link an archive member needed but not an object
    ERR_LINK_MACHINE_MISMATCH,             // can't link an object for another machine than the output's
    ERR_LINK_MACHINE_NOT_SUPPORTED,        // can't apply relocations for a machine the linker does not know
    ERR_LINK_OBJECT_NOT_RELOCATABLE,       // can't link a shared object or an executable as a relocatable object

    ERR_LOAD_NOT_ELF,                      // can't load a file that is not ELF
    ERR_LOAD_NOT_ELF64_LSB,                // can't load an ELF file that is not 64-bit little-endian
    ERR_LOAD_NOT_EXECUTABLE,               // can't load an ELF file that is not an executable
    ERR_LOAD_NO_SEGMENTS,                  // can't load an executable with no loadable segments
    ERR_LOAD_SEGMENT_TRUNCATED,            // can't load a segment that runs past the end of its file
    ERR_LOAD_ARGS_TOO_LARGE,               // can't fit the arguments and environment in a quarter of the stack

    ERR_STR_REGEX_NOT_COMPILED,            // can't compile a regex pattern

    ERR_TXT_OPERAND_NOT_KNOWN,             // can't read an operand of a kind the assembler does not know
    ERR_TXT_INSTRUCTION_NOT_KNOWN,         // can't assemble an instruction the encoder has no form for
    ERR_TXT_DATA_NOT_KNOWN,                // can't give a data directive an item that is neither a number nor, in `.quad`, an address
    ERR_TXT_DIRECTIVE_NOT_KNOWN,           // can't read a directive the assembler does not know, or its arguments
    ERR_TXT_STRING_NOT_TERMINATED,         // can't leave a string without its closing quote

    ERR_CPU_READ_NOT_MAPPED,               // can't read unmapped memory
    ERR_CPU_WRITE_NOT_MAPPED,              // can't write to unmapped memory
    ERR_CPU_FETCH_NOT_MAPPED,              // can't run an instruction from unmapped memory
    ERR_CPU_OPCODE_NOT_DECODABLE,          // can't run an instruction that does not decode
    ERR_CPU_OPCODE_NOT_IMPLEMENTED,        // can't run a one-byte opcode the CPU does not implement
    ERR_CPU_TWO_BYTE_NOT_IMPLEMENTED,      // can't run a two-byte opcode the CPU does not implement
    ERR_CPU_GROUP1_NOT_IMPLEMENTED,        // can't run a group 1 operation the CPU does not implement
    ERR_CPU_GROUP2_NOT_IMPLEMENTED,        // can't run a group 2 operation the CPU does not implement
    ERR_CPU_GROUP3_NOT_IMPLEMENTED,        // can't run a group 3 operation the CPU does not implement
    ERR_CPU_GROUP5_NOT_IMPLEMENTED,        // can't run a group 5 operation the CPU does not implement
    ERR_CPU_SSE_DOUBLE_NOT_IMPLEMENTED,    // can't run a scalar double SSE operation the CPU does not implement
    ERR_CPU_SSE_SINGLE_NOT_IMPLEMENTED,    // can't run a scalar single SSE operation the CPU does not implement
    ERR_CPU_X87_MEM_NOT_IMPLEMENTED,       // can't run an x87 memory operation the CPU does not implement
    ERR_CPU_X87_D9_NOT_IMPLEMENTED,        // can't run a D9 x87 operation the CPU does not implement
    ERR_CPU_X87_DE_NOT_IMPLEMENTED,        // can't run a DE x87 operation the CPU does not implement
    ERR_CPU_X87_DF_NOT_IMPLEMENTED,        // can't run a DF x87 operation the CPU does not implement
    ERR_CPU_X87_DD_NOT_IMPLEMENTED,        // can't run a DD x87 operation the CPU does not implement
    ERR_CPU_X87_NOT_IMPLEMENTED,           // can't run an x87 opcode the CPU does not implement
    ERR_CPU_DIVIDE_BY_ZERO,                // can't divide by zero
    ERR_CPU_QUOTIENT_TOO_LARGE,            // can't divide when the quotient overflows its register

    ERR_CC_ARCH_NOT_SUPPORTED,             // can't target an architecture other than x86_64
    ERR_CC_OUTPUT_NOT_WRITEABLE,           // can't create the compiler's output file
    ERR_CC_RUNTIME_NOT_FOUND,              // can't link without finding the runtime directory
    ERR_CC_STD_NOT_SUPPORTED,              // can't compile to a standard other than C99
    ERR_CC_TARGET_NOT_SUPPORTED,           // can't target a runtime other than linux or ivanemu

    ERR_AS_INPUT_NOT_READABLE,             // can't read the assembler's input file
    ERR_AS_OUTPUT_NOT_WRITEABLE,           // can't create the assembler's object file
    ERR_AS_OUTPUT_WRITE_FAILED,            // can't write or close the assembler's object file

    ERR_AR_ARCHIVE_NOT_READABLE,           // can't read the archive
    ERR_AR_NOT_ARCHIVE,                    // can't read a file that does not open with `!<arch>` as an archive
    ERR_AR_MEMBER_TRUNCATED,               // can't read a member that runs past the end of its archive
    ERR_AR_HEADER_MALFORMED,               // can't read a member header whose size or terminator is malformed
    ERR_AR_NAME_NOT_FOUND,                 // can't read a long member name the long-name table does not hold
    ERR_AR_INPUT_NOT_READABLE,             // can't add a file that can't be read
    ERR_AR_MEMBER_NOT_FOUND,               // can't find a member the archive does not hold
    ERR_AR_EXTRACT_NAME_NOT_PLAIN,         // can't extract a member whose name holds a slash
    ERR_AR_EXTRACT_NOT_WRITEABLE,          // can't write an extracted member
    ERR_AR_OUTPUT_NOT_WRITEABLE,           // can't write the archive

    ERR_LD_PLACE_MALFORMED,                // can't pass `-place` anything but SEC@ADDR
    ERR_LD_OUTPUT_NOT_WRITEABLE,           // can't write the linker's executable

    ERR_EMU_ARCH_NOT_SUPPORTED,            // can't emulate an architecture other than x86_64
    ERR_EMU_PROGRAM_NOT_READABLE,          // can't read the executable to run
    ERR_EMU_ARCH_NOT_X86_64,               // can't run an executable that is not for x86_64

    ERR_CODE_COUNT
};

// How a diagnostic is reported by default.
typedef enum Err_Level Err_Level;
enum Err_Level {
    ERR_LEVEL_FATAL,                       // an error that stays one
    ERR_LEVEL_ERROR,                       // an error that may become a warning
    ERR_LEVEL_PEDANTIC,                    // a warning that -pedantic-errors makes an error
    ERR_LEVEL_WARNING,                     // a warning that -Werror makes an error
    ERR_LEVEL_COUNT
};

// A diagnostic's level, name and message format.
typedef struct Err_Entry Err_Entry;
struct Err_Entry {
    Err_Level   ee_level;
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
