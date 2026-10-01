/*
 * C source file for the parser's declarator and parameter helpers.
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
#include "lang/par.h"

// State for the function definition currently being parsed.
static char     *Par_CurFuncName;
static bool      Par_CurStatic;
static bool      Par_InFunction;
static Ast_Var  *Par_CurFuncVar;
static Ast_Node *Par_CurSizes;

// Serial number of the next compound literal's object.
static int32_t Par_CompoundCount;

// Serial number of the next block-scope static's symbol.
static int32_t Par_StaticCount;

// Serial number of the next local a variable-length array's size lands in.
static int32_t Par_SizeCount;

// The unsized array an initializer is sizing, and the length it has reached.
static Ast_Type *Par_OpenArray;
static int32_t   Par_OpenLen;

// The type and storage class one declaration's declarators share.
static Ast_Type   *Par_DeclType;
static Ast_Storage Par_DeclStorage;
static char       *Par_DeclName;
static Ast_Line    Par_DeclLine;

// The type the top-level declarator just read works out to.
static Ast_Type   *Par_CurDeclType;

// The program assembled so far.
static Ast_Func *Par_ProgHead;
static Ast_Func *Par_ProgTail;

// Value the next enumerator takes.
static int64_t Par_EnumValue;

// The record __builtin_va_list names.
static Ast_Type *Par_VaList;

// Record the specifier one declaration's declarators share.
void Par_SetDeclSpec(const Par_Specs *specs, Par_StorageUse use, Ast_Line line)
{
    Par_DeclStorage = Par_SpecsStorage(specs, use, line);
    Par_DeclType    = Par_SpecsType(specs, line);
}

// Start an enumerator list over.
void Par_ResetEnum(void)
{
    Par_EnumValue = 0;
}

// Empty a parameter list.
void Par_ClearParams(Par_ParamList *list)
{
    list->pl_head  = NULL;
    list->pl_tail  = NULL;
    list->pl_count = 0;
    list->pl_va    = AST_TYPE_FIXED;
    list->pl_proto = AST_TYPE_NOPROTO;
}

// Append one parameter to a list.
void Par_PushParam(Par_ParamList *list, Ast_Var *var)
{
    if (! var) {
        return;
    }
    var->av_param_next = NULL;
    if (list->pl_tail) {
        list->pl_tail->av_param_next = var;
    } else {
        list->pl_head = var;
    }
    list->pl_tail = var;
    list->pl_count++;
}

// Start a declarator for name.
Par_Decl *Par_NewDecl(char *name)
{
    Par_Decl *decl = calloc(1, sizeof(Par_Decl));
    decl->pc_name = name;
    return decl;
}

// Reject a declarator with no name.
void Par_NeedName(Par_Decl *decl, Ast_Line line)
{
    Err_AssertAt(decl->pc_line ? decl->pc_line : line, decl->pc_name, ERR_PAR_DECL_NOT_NAMED);
}

// Append one derivation to a declarator.
Par_Deriv *Par_AddDeriv(Par_Decl *decl, Par_DerivKind kind, Ast_Line line)
{
    Par_Deriv *deriv = calloc(1, sizeof(Par_Deriv));
    deriv->pd_kind = kind;
    deriv->pd_line = line;
    if (decl->pc_tail) {
        decl->pc_tail->pd_next = deriv;
    } else {
        decl->pc_head = deriv;
    }
    decl->pc_tail = deriv;
    return deriv;
}

// Append a run of stars to a declarator, the star nearest the name first.
void Par_AddPointers(Par_Decl *decl, const Par_Deriv *star)
{
    if (! star) {
        return;
    }
    Par_AddPointers(decl, star->pd_next);
    Par_AddDeriv(decl, PAR_DERIV_POINTER, star->pd_line)->pd_qual = star->pd_qual;
}

// Give an array derivation the length an expression computes.
void Par_SetArrayLen(Par_Deriv *deriv, Ast_Node *len)
{
    if (! Sem_Fold(len, &deriv->pd_len)) {
        deriv->pd_vlen = len;
    }
}

// Wrap base in one derivation list, outermost first.
Ast_Type *Par_ApplyDerivs(Ast_Type *base, Par_Deriv *deriv)
{
    if (! deriv) {
        return base;
    }
    Ast_Type *inner = Par_ApplyDerivs(base, deriv->pd_next);
    switch (deriv->pd_kind) {
        case PAR_DERIV_POINTER: {
            return Ast_Qualify(Ast_NewPointer(inner), deriv->pd_qual);
        }
        case PAR_DERIV_ARRAY: {
            Err_AssertAt(deriv->pd_line, inner->at_kind != AST_TYPE_KIND_FUNC, ERR_PAR_ARRAY_OF_FUNCTIONS);
            Err_AssertAt(deriv->pd_line, ! deriv->pd_decor, ERR_PAR_ARRAY_DECOR_NOT_PARAM);
            if (deriv->pd_empty) {
                return Ast_NewUnsizedArray(inner);
            }
            if (deriv->pd_star) {
                return Ast_NewVla(inner, Ast_NewNode(AST_NODE_KIND_NOP, deriv->pd_line));
            }
            if (deriv->pd_vlen) {
                return Ast_NewVla(inner, deriv->pd_vlen);
            }
            Err_AssertAt(deriv->pd_line, deriv->pd_len > 0, ERR_PAR_ARRAY_LEN_NOT_POSITIVE);
            if (Ast_IsVla(inner)) {
                return Ast_NewVla(inner, Ast_NewNum(deriv->pd_len, deriv->pd_line));
            }
            return Ast_NewArray(inner, (int32_t) deriv->pd_len);
        }
        case PAR_DERIV_FUNCTION: {
            Err_AssertAt(deriv->pd_line, inner->at_kind != AST_TYPE_KIND_FUNC && inner->at_kind != AST_TYPE_KIND_ARRAY, ERR_PAR_FUNCTION_BAD_RETURN);
            Par_NeedFixedSize(inner, deriv->pd_line);
            return Ast_NewFunction(inner, deriv->pd_params.pl_head, deriv->pd_params.pl_count, deriv->pd_params.pl_va, deriv->pd_params.pl_proto);
        }
        case PAR_DERIV_COUNT: {
            // empty
        } break;
    }
    return inner;
}

// Give a declarator the type applying it to base yields.
Ast_Type *Par_ApplyDecl(Ast_Type *base, Par_Decl *decl)
{
    return Par_ApplyDerivs(base, decl->pc_head);
}

// Decay a parameter's array or function type to a pointer.
Ast_Type *Par_AdjustParam(Ast_Type *type)
{
    if (type->at_kind == AST_TYPE_KIND_ARRAY) {
        return Ast_NewPointer(type->at_base);
    }
    if (type->at_kind == AST_TYPE_KIND_FUNC) {
        return Ast_NewPointer(type);
    }
    return type;
}

// Accept `static` and qualifiers on a parameter's outermost array.
void Par_TakeArrayDecor(Par_Decl *decl, Ast_Line line)
{
    for (Par_Deriv *deriv = decl->pc_head; deriv; deriv = deriv->pd_next) {
        if (! deriv->pd_decor) {
            continue;
        }
        Err_AssertAt(line, deriv == decl->pc_head && deriv->pd_kind == PAR_DERIV_ARRAY, ERR_PAR_ARRAY_DECOR_NOT_OUTERMOST);
        Err_AssertAt(line, ! (deriv->pd_decor & PAR_ARRAY_STATIC) || (! deriv->pd_empty && ! deriv->pd_star), ERR_PAR_ARRAY_STATIC_NO_LEN);
        deriv->pd_decor = PAR_ARRAY_NONE;
    }
}

// Build one named parameter.
Ast_Var *Par_MakeParam(Ast_Type *base, Par_Decl *decl, Ast_Line line)
{
    Par_TakeArrayDecor(decl, line);
    Ast_Type *type = Par_ApplyDecl(base, decl);
    Ast_Var *var = calloc(1, sizeof(Ast_Var));

    var->av_name = decl->pc_name;
    var->av_type = Par_AdjustParam(type);
    var->av_line = line;
    Par_KeepVmType(var, type);
    if (var->av_name) {
        Ast_DeclarePrototypeParam(var);
    }
    return var;
}

// Build one old-style parameter.
Ast_Var *Par_MakeKnrParam(char *name, Ast_Line line)
{
    Ast_Var *var = calloc(1, sizeof(Ast_Var));

    var->av_name = name;
    var->av_line = line;
    return var;
}

// Give an old-style parameter the type its declaration list names.
void Par_SetKnrParam(Par_Decl *decl, Ast_Line line)
{
    Par_NeedName(decl, line);
    Par_TakeArrayDecor(decl, line);
    for (Ast_Var *param = Par_CurDeclType->at_params; param; param = param->av_param_next) {
        if (Str_Equals(param->av_name, decl->pc_name)) {
            Ast_Type *type = Par_ApplyDecl(Par_DeclType, decl);
            param->av_type = Par_AdjustParam(type);
            Par_KeepVmType(param, type);
            return;
        }
    }
    Err_RaiseAt(line, ERR_PAR_KNR_NOT_PARAM, decl->pc_name);
}

// Reject an old-style parameter the declaration list never typed.
void Par_CheckKnrParams(void)
{
    for (Ast_Var *param = Par_CurDeclType->at_params; param; param = param->av_param_next) {
        Err_AssertAt(param->av_line, param->av_type, ERR_PAR_KNR_NOT_DECLARED, param->av_name);
    }
}

// Build one unnamed parameter.
Ast_Var *Par_MakeAnonParam(Ast_Type *type, Ast_Line line)
{
    if (type->at_kind == AST_TYPE_KIND_VOID) {
        return NULL;
    }
    Ast_Var *var = calloc(1, sizeof(Ast_Var));
    var->av_type = Par_AdjustParam(type);
    var->av_line = line;
    Par_KeepVmType(var, type);
    return var;
}

// Keep a variably modified parameter's type for sizing on entry.
void Par_KeepVmType(Ast_Var *param, Ast_Type *type)
{
    if (Ast_IsVm(type)) {
        param->av_vmtype = type;
    }
}

// Size each variably modified parameter's lengths, outermost included.
Ast_Node *Par_SizeParams(void)
{
    Ast_Node *sizes = NULL;

    for (Ast_Var *param = Par_CurDeclType->at_params; param; param = param->av_param_next) {
        Ast_Node *size = param->av_vmtype ? Par_SizeExpr(param->av_vmtype, param->av_line) : NULL;
        if (size) {
            sizes = sizes ? Ast_NewBinary(AST_NODE_KIND_COMMA, sizes, size, param->av_line) : size;
        }
    }
    return sizes;
}

// Finish the parameters before a function body opens.
void Par_BeginBody(void)
{
    Err_AssertAt(Par_DeclLine, Par_InFunction, ERR_PAR_BODY_NOT_FUNCTION, Par_DeclName);
    Par_CheckKnrParams();
    Par_CurSizes = Par_SizeParams();
}

// Give an integer literal the type its spelling and value ask for.
Par_Num Par_NumLiteral(const char *text)
{
    const char *suffix = text;
    Ast_TypeSign sign = AST_TYPE_SIGNED;
    Ast_TypeKind least = AST_TYPE_KIND_INT;

    bool decimal = text[0] != '0';
    uint64_t val = strtoull(text, (char **) &suffix, 0);

    for (const char *ptr = suffix; *ptr; ptr++) {
        if (*ptr == 'u' || *ptr == 'U') {
            sign = AST_TYPE_UNSIGNED;
        } else if (least == AST_TYPE_KIND_LONG) {
            least = AST_TYPE_KIND_LLONG;
        } else {
            least = AST_TYPE_KIND_LONG;
        }
    }

    for (Ast_TypeKind kind = least; kind <= AST_TYPE_KIND_LAST_INT; kind++) {
        Ast_Type *type = Ast_IntegerType(kind, sign);
        int32_t bits = type->at_size * AST_BITS_PER_BYTE;
        uint64_t room;

        if (type->at_sign == AST_TYPE_UNSIGNED) {
            room = ~(uint64_t) 0 >> (PAR_VALUE_BITS - bits);
        } else {
            room = ~(uint64_t) 0 >> (PAR_VALUE_BITS - bits + 1);
        }
        if (val <= room) {
            return (Par_Num) {
                .pn_val  = (int64_t) val,
                .pn_type = type
            };
        }
        if (! decimal && type->at_sign != AST_TYPE_UNSIGNED) {
            Ast_Type *alt = Ast_IntegerType(kind, AST_TYPE_UNSIGNED);
            if (val <= ~(uint64_t) 0 >> (PAR_VALUE_BITS - alt->at_size * AST_BITS_PER_BYTE)) {
                return (Par_Num) {
                    .pn_val  = (int64_t) val,
                    .pn_type = alt
                };
            }
        }
    }
    return (Par_Num) {
        .pn_val  = (int64_t) val,
        .pn_type = Ast_IntegerType(AST_TYPE_KIND_LLONG, AST_TYPE_UNSIGNED)
    };
}

// Give a floating literal the value and type its spelling asks for.
Par_FNum Par_FloatLiteral(const char *text)
{
    char last = text[strlen(text) - 1];

    if (last == 'f' || last == 'F') {
        return (Par_FNum) {
            .pf_val  = strtof(text, NULL),
            .pf_type = &Ast_TypeFloat
        };
    }
    if (last == 'l' || last == 'L') {
        return (Par_FNum) {
            .pf_val  = strtold(text, NULL),
            .pf_type = &Ast_TypeLDouble
        };
    }
    return (Par_FNum) {
        .pf_val  = strtod(text, NULL),
        .pf_type = &Ast_TypeDouble
    };
}

// Decode a character literal body into its value and type.
Par_Num Par_CharLiteral(const char *body, size_t len, size_t width, Ast_Line line)
{
    int64_t value = 0;
    size_t bytes = 0;
    char *data = Par_UnescapeLiteral(body, len, width, &bytes, line);

    if (width > AST_TYPE_SIZE_CHAR) {
        value = (int32_t) Par_GetElement(data + bytes - width, width);
    } else if (bytes == 1) {
        value = (int8_t) data[0];
    } else {
        for (size_t i = 0; i < bytes; i++) {
            value = (int32_t) ((value << AST_BITS_PER_BYTE) | (uint8_t) data[i]);
        }
    }
    Str_Free(data);
    return (Par_Num) {
        .pn_val  = value,
        .pn_type = &Ast_TypeInt
    };
}

// Decode a string literal body into its elements.
Ast_Str Par_StringLiteral(const char *body, size_t len, size_t width, Ast_Line line)
{
    Ast_Str str;

    str.as_width = width;
    str.as_data  = Par_UnescapeLiteral(body, len, width, &str.as_len, line);
    return str;
}

// Re-encode a string literal into elements of the given width.
Ast_Str Par_WidenString(Ast_Str str, size_t width)
{
    size_t n = 0;
    Ast_Str out;

    out.as_data  = calloc(str.as_len / str.as_width + 1, width);
    out.as_width = width;
    for (size_t i = 0; i < str.as_len; i += str.as_width) {
        Par_PutElement(out.as_data, &n, width, Par_GetElement(str.as_data + i, str.as_width));
    }
    out.as_len = n;
    return out;
}

// Join two adjacent string literals into one.
Ast_Str Par_ConcatStrings(Ast_Str left, Ast_Str right)
{
    size_t width = left.as_width > right.as_width ? left.as_width : right.as_width;
    Ast_Str a = Par_WidenString(left, width);
    Ast_Str b = Par_WidenString(right, width);
    Ast_Str out;

    out.as_data  = calloc(a.as_len + b.as_len + width, 1);
    out.as_len   = a.as_len + b.as_len;
    out.as_width = width;
    memcpy(out.as_data, a.as_data, a.as_len);
    memcpy(out.as_data + a.as_len, b.as_data, b.as_len);
    Str_Free(a.as_data);
    Str_Free(b.as_data);
    Str_Free(left.as_data);
    Str_Free(right.as_data);
    return out;
}

// Return the value of a digit, or -1 when it is not one.
int32_t Par_DigitValue(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + PAR_BASE_DECIMAL;
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + PAR_BASE_DECIMAL;
    }
    return -1;
}

// Scan up to count digits of the given base, advancing the position.
uint64_t Par_ScanDigits(const char *text, size_t len, size_t *pos, Par_Base base, size_t count)
{
    uint64_t value = 0;

    for (size_t n = 0; n < count && *pos < len; n++) {
        int32_t digit = Par_DigitValue(text[*pos]);
        if (digit < 0 || digit >= (int32_t) base) {
            break;
        }
        value = value * (uint64_t) base + (uint64_t) digit;
        (*pos)++;
    }
    return value;
}

// Read one little-endian element of width bytes.
uint64_t Par_GetElement(const char *data, size_t width)
{
    uint64_t value = 0;

    for (size_t i = 0; i < width; i++) {
        value |= (uint64_t) (uint8_t) data[i] << (i * AST_BITS_PER_BYTE);
    }
    return value;
}

// Append one little-endian element of width bytes holding value.
void Par_PutElement(char *buf, size_t *len, size_t width, uint64_t value)
{
    for (size_t i = 0; i < width; i++) {
        buf[(*len)++] = (char) (value >> (i * AST_BITS_PER_BYTE));
    }
}

// Append the element an escape sequence stands for.
void Par_PutEscape(char *buf, size_t *len, size_t width, uint64_t value, Ast_Line line)
{
    uint64_t room = ~(uint64_t) 0 >> (PAR_VALUE_BITS - width * AST_BITS_PER_BYTE);

    Err_AssertAt(line, value <= room, ERR_PAR_ESCAPE_OUT_OF_RANGE, width);
    Par_PutElement(buf, len, width, value);
}

// Append a code point as its UTF-8 bytes.
void Par_PutUtf8(char *buf, size_t *len, uint64_t value)
{
    if (value < PAR_UTF8_MAX_ONE) {
        Par_PutElement(buf, len, AST_TYPE_SIZE_CHAR, value);
        return;
    }

    size_t n = PAR_UTF8_LEN_TWO;

    if (value >= PAR_UTF8_MAX_THREE) {
        n = PAR_UTF8_LEN_FOUR;
    } else if (value >= PAR_UTF8_MAX_TWO) {
        n = PAR_UTF8_LEN_THREE;
    }

    uint64_t lead = (PAR_BYTE_MASK << (AST_BITS_PER_BYTE - n)) & PAR_BYTE_MASK;

    Par_PutElement(buf, len, AST_TYPE_SIZE_CHAR, lead | (value >> ((n - 1) * PAR_UTF8_SHIFT)));
    for (size_t k = n - 1; k > 0; k--) {
        Par_PutElement(buf, len, AST_TYPE_SIZE_CHAR, PAR_UTF8_CONT | ((value >> ((k - 1) * PAR_UTF8_SHIFT)) & PAR_UTF8_MASK));
    }
}

// Decode a literal body into elements of width bytes.
char *Par_UnescapeLiteral(const char *body, size_t len, size_t width, size_t *out_len, Ast_Line line)
{
    size_t n = 0;
    char *buf = calloc(len + 1, width);

    for (size_t i = 0; i < len; i++) {
        if (body[i] != '\\' || i + 1 == len) {
            Par_PutElement(buf, &n, width, (uint8_t) body[i]);
            continue;
        }

        size_t pos = i + 2;

        switch (body[i + 1]) {
            case 'a': {
                Par_PutElement(buf, &n, width, '\a');
            } break;
            case 'b': {
                Par_PutElement(buf, &n, width, '\b');
            } break;
            case 'f': {
                Par_PutElement(buf, &n, width, '\f');
            } break;
            case 'n': {
                Par_PutElement(buf, &n, width, '\n');
            } break;
            case 'r': {
                Par_PutElement(buf, &n, width, '\r');
            } break;
            case 't': {
                Par_PutElement(buf, &n, width, '\t');
            } break;
            case 'v': {
                Par_PutElement(buf, &n, width, '\v');
            } break;
            case '\\': {
                Par_PutElement(buf, &n, width, '\\');
            } break;
            case '\'': {
                Par_PutElement(buf, &n, width, '\'');
            } break;
            case '"': {
                Par_PutElement(buf, &n, width, '"');
            } break;
            case '?': {
                Par_PutElement(buf, &n, width, '?');
            } break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7': {
                pos = i + 1;
                Par_PutEscape(buf, &n, width, Par_ScanDigits(body, len, &pos, PAR_BASE_OCTAL, PAR_MAX_OCTAL_DIGITS), line);
            } break;
            case 'x': {
                size_t start = pos;
                uint64_t value = Par_ScanDigits(body, len, &pos, PAR_BASE_HEX, PAR_MAX_HEX_DIGITS);
                Err_AssertAt(line, pos != start, ERR_PAR_ESCAPE_HEX_EMPTY);
                Par_PutEscape(buf, &n, width, value, line);
            } break;
            case 'u':
            case 'U': {
                size_t count = body[i + 1] == 'u' ? PAR_UCN_SHORT_DIGITS : PAR_UCN_LONG_DIGITS;
                size_t start = pos;
                uint64_t value = Par_ScanDigits(body, len, &pos, PAR_BASE_HEX, count);
                Err_AssertAt(line, pos - start == count, ERR_PAR_ESCAPE_UCN_NOT_COMPLETE);
                if (width > AST_TYPE_SIZE_CHAR) {
                    Par_PutEscape(buf, &n, width, value, line);
                } else {
                    Par_PutUtf8(buf, &n, value);
                }
            } break;
            default: {
                Err_RaiseAt(line, ERR_PAR_ESCAPE_NOT_KNOWN, body[i + 1]);
            } break;
        }
        i = pos - 1;
    }

    *out_len = n;
    return buf;
}

// Empty a specifier set.
void Par_ClearSpecs(Par_Specs *specs)
{
    specs->ps_specs    = 0;
    specs->ps_qual     = 0;
    specs->ps_type     = NULL;
    specs->ps_storage  = AST_STORAGE_NONE;
    specs->ps_nstorage = 0;
    specs->ps_register = false;
    specs->ps_inline   = false;
}

// Return the specifier set one storage-class keyword makes.
Par_Specs Par_StorageSpec(Ast_Storage storage)
{
    Par_Specs specs;

    Par_ClearSpecs(&specs);
    specs.ps_storage  = storage;
    specs.ps_nstorage = 1;
    return specs;
}

// Add one type specifier keyword to a declaration's set.
Par_Spec Par_AddSpec(Par_Spec specs, Par_Spec spec, Ast_Line line)
{
    if (spec == PAR_SPEC_LONG && (specs & PAR_SPEC_LONG)) {
        spec = PAR_SPEC_LLONG;
    }
    Err_AssertAt(line, ! (specs & spec), ERR_PAR_SPEC_REPEATED);
    return specs | spec;
}

// Merge one specifier or qualifier into a declaration's set.
void Par_TakeSpec(Par_Specs *into, const Par_Specs *one, Ast_Line line)
{
    bool clash = (one->ps_type && (into->ps_type || into->ps_specs)) || (one->ps_specs && into->ps_type);

    Err_AssertAt(line, ! clash, ERR_PAR_SPEC_TWO_TYPES);
    if (one->ps_specs) {
        into->ps_specs = Par_AddSpec(into->ps_specs, one->ps_specs, line);
    }
    if (one->ps_type) {
        into->ps_type = one->ps_type;
    }
    if (one->ps_nstorage) {
        Err_AssertAt(line, ! into->ps_nstorage, ERR_PAR_STORAGE_REPEATED);
        into->ps_storage  = one->ps_storage;
        into->ps_nstorage = one->ps_nstorage;
        into->ps_register = one->ps_register;
    }
    into->ps_qual   |= one->ps_qual;
    into->ps_inline |= one->ps_inline;
}

// Return the storage class a declaration's specifiers name.
Ast_Storage Par_SpecsStorage(const Par_Specs *specs, Par_StorageUse use, Ast_Line line)
{
    bool storage = specs->ps_nstorage && ! (use == PAR_STORAGE_REGISTER && specs->ps_register);

    if (use != PAR_STORAGE_ANY) {
        Err_AssertAt(line, ! storage && ! specs->ps_inline, ERR_PAR_STORAGE_NOT_ALLOWED);
    }
    return specs->ps_storage;
}

// Return the type a declaration's specifier keywords name.
Ast_Type *Par_SpecType(Par_Spec specs, Ast_Line line)
{
    int32_t base = specs & ~(PAR_SPEC_SIGNED | PAR_SPEC_UNSIGNED);
    bool floating = base == PAR_SPEC_FLOAT || base == PAR_SPEC_DOUBLE || base == (PAR_SPEC_LONG | PAR_SPEC_DOUBLE);
    Par_Spec explicit = specs & (PAR_SPEC_SIGNED | PAR_SPEC_UNSIGNED);
    Ast_TypeSign sign = specs & PAR_SPEC_UNSIGNED ? AST_TYPE_UNSIGNED : AST_TYPE_SIGNED;

    Err_AssertAt(line, ! explicit || ! floating, ERR_PAR_FLOAT_SIGNED);
    switch (base) {
        case PAR_SPEC_VOID: {
            Err_AssertAt(line, ! explicit, ERR_PAR_VOID_SIGNED);
            return &Ast_TypeVoid;
        } break;
        case PAR_SPEC_BOOL: {
            Err_AssertAt(line, ! explicit, ERR_PAR_BOOL_SIGNED);
            return &Ast_TypeBool;
        } break;
        case PAR_SPEC_CHAR: {
            return Ast_IntegerType(AST_TYPE_KIND_CHAR, sign);
        } break;
        case PAR_SPEC_SHORT:
        case PAR_SPEC_SHORT | PAR_SPEC_INT: {
            return Ast_IntegerType(AST_TYPE_KIND_SHORT, sign);
        } break;
        case PAR_SPEC_INT: {
            return Ast_IntegerType(AST_TYPE_KIND_INT, sign);
        } break;
        case PAR_SPEC_NONE: {
            Err_AssertAt(line, explicit, ERR_PAR_SPEC_MISSING);
            return Ast_IntegerType(AST_TYPE_KIND_INT, sign);
        } break;
        case PAR_SPEC_LONG:
        case PAR_SPEC_LONG | PAR_SPEC_INT: {
            return Ast_IntegerType(AST_TYPE_KIND_LONG, sign);
        } break;
        case PAR_SPEC_LONG | PAR_SPEC_LLONG:
        case PAR_SPEC_LONG | PAR_SPEC_LLONG | PAR_SPEC_INT: {
            return Ast_IntegerType(AST_TYPE_KIND_LLONG, sign);
        } break;
        case PAR_SPEC_FLOAT: {
            return &Ast_TypeFloat;
        } break;
        case PAR_SPEC_DOUBLE: {
            return &Ast_TypeDouble;
        } break;
        case PAR_SPEC_LONG | PAR_SPEC_DOUBLE: {
            return &Ast_TypeLDouble;
        } break;
        default: {
            Err_RaiseAt(line, ERR_PAR_SPEC_NOT_VALID);
        }
    }
    return &Ast_TypeInt;
}

// Return the type one declaration's specifiers name.
Ast_Type *Par_SpecsType(const Par_Specs *specs, Ast_Line line)
{
    Ast_Type *type = specs->ps_type ? specs->ps_type : Par_SpecType(specs->ps_specs, line);
    return Ast_Qualify(type, specs->ps_qual);
}

// The type __builtin_va_list names.
Ast_Type *Par_VaListType(void)
{
    if (Par_VaList) {
        return Par_VaList;
    }

    Ast_Member *gp = Ast_NewMember("gp_offset", &Ast_TypeInt, 0);
    gp->am_next = Ast_NewMember("fp_offset", &Ast_TypeInt, 0);
    gp->am_next->am_next = Ast_NewMember("overflow_arg_area", Ast_NewPointer(&Ast_TypeVoid), 0);
    gp->am_next->am_next->am_next = Ast_NewMember("reg_save_area", Ast_NewPointer(&Ast_TypeVoid), 0);

    Ast_Type *tag = Ast_NewAggregate(AST_TYPE_KIND_STRUCT, "__va_list_tag");
    Ast_LayoutAggregate(tag, gp, 0);
    Par_VaList = Ast_NewArray(tag, 1);
    return Par_VaList;
}

// Build the node reading the next anonymous argument.
Ast_Node *Par_VaArg(Ast_Node *ap, Ast_Type *type, Ast_Line line)
{
    bool object = type->at_kind != AST_TYPE_KIND_VOID && type->at_kind != AST_TYPE_KIND_FUNC && type->at_complete == AST_TYPE_COMPLETE;

    Err_AssertAt(line, object && type->at_kind != AST_TYPE_KIND_ARRAY, ERR_PAR_VA_ARG_TYPE);
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_VA_ARG, ap, line);
    node->an_type = type;
    return Par_WithSizes(type, node, line);
}

// Build the copy of one argument cursor into another.
Ast_Node *Par_VaCopy(Ast_Node *dst, Ast_Node *src, Ast_Line line)
{
    Ast_Node *to = Ast_NewUnary(AST_NODE_KIND_DEREF, dst, line);
    Ast_Node *from = Ast_NewUnary(AST_NODE_KIND_DEREF, src, line);
    return Ast_NewBinary(AST_NODE_KIND_ASSIGN, to, from, line);
}

// Join two member lists, keeping declaration order.
Ast_Member *Par_AppendMembers(Ast_Member *head, Ast_Member *tail)
{
    if (! head) {
        return tail;
    }
    Ast_Member *last = head;
    while (last->am_next) {
        last = last->am_next;
    }
    last->am_next = tail;
    return head;
}

// Narrow a member to the bits a `: width` gave it.
void Par_AddBitfield(Ast_Member *member, Ast_Node *width, Ast_Line line)
{
    int64_t bits = 0;

    Err_AssertAt(line, Sem_Fold(width, &bits), ERR_PAR_BITFIELD_NOT_CONSTANT);
    Err_AssertAt(line, Ast_IsInteger(member->am_type), ERR_PAR_BITFIELD_NOT_INTEGER);
    Err_AssertAt(line, bits >= 0, ERR_PAR_BITFIELD_NEGATIVE);
    Err_AssertAt(line, bits <= member->am_type->at_size * AST_BITS_PER_BYTE, ERR_PAR_BITFIELD_TOO_WIDE);
    Err_AssertAt(line, bits != 0 || ! member->am_name, ERR_PAR_BITFIELD_NAMED_ZERO);
    member->am_bits = (int32_t) bits;
}

// Turn one member declaration's declarators into members of the shared type.
Ast_Member *Par_MakeMembers(Ast_Type *type, Par_Decl *decls)
{
    Ast_Member head = {0};
    Ast_Member *tail = &head;

    for (Par_Decl *decl = decls; decl; decl = decl->pc_next) {
        Err_AssertAt(decl->pc_line, decl->pc_name || decl->pc_bits, ERR_PAR_MEMBER_NOT_NAMED);
        tail->am_next = Ast_NewMember(decl->pc_name, Par_ApplyDecl(type, decl), decl->pc_line);
        tail = tail->am_next;
        Par_NeedFixedSize(tail->am_type, decl->pc_line);
        if (decl->pc_head && decl->pc_head->pd_kind == PAR_DERIV_ARRAY && decl->pc_head->pd_empty) {
            tail->am_flexible = true;
        }
        if (decl->pc_bits) {
            Par_AddBitfield(tail, decl->pc_bits, decl->pc_line);
        }
    }
    return head.am_next;
}

// Reject a tag already bound to another kind of aggregate.
void Par_CheckTagKind(const Ast_Type *type, Ast_TypeKind kind, const char *tag, Ast_Line line)
{
    Err_AssertAt(line, ! type || type->at_kind == kind, ERR_PAR_TAG_WRONG_KIND, tag);
}

// Open a struct or union definition, binding its tag first.
Ast_Type *Par_BeginAggregate(Ast_TypeKind kind, const char *tag, Ast_Line line)
{
    Ast_Type *type = tag ? Ast_FindTagHere(tag) : NULL;

    Err_AssertAt(line, ! type || ! type->at_complete, ERR_PAR_TAG_REDEFINED, tag);
    Par_CheckTagKind(type, kind, tag, line);
    if (! type) {
        type = Ast_NewAggregate(kind, tag);
        if (tag) {
            Ast_DeclareTag(tag, type);
        }
    }
    return type;
}

// Name a struct or union not yet defined.
Ast_Type *Par_ReferenceAggregate(Ast_TypeKind kind, const char *tag, Ast_Line line)
{
    Ast_Type *type = Ast_FindTag(tag);

    Par_CheckTagKind(type, kind, tag, line);
    if (! type) {
        type = Ast_NewAggregate(kind, tag);
        Ast_DeclareTag(tag, type);
    }
    return type;
}

// Declare one enumeration constant and step the next one's value.
void Par_AddEnumConst(const char *name, Ast_Node *value, Ast_Line line)
{
    Err_AssertAt(line, ! value || Sem_Fold(value, &Par_EnumValue), ERR_PAR_ENUM_NOT_CONSTANT, name);
    Err_AssertAt(line, ! Ast_IsEnumConstHere(name) && ! Ast_FindVarHere(name) && ! Ast_FindTypedefHere(name), ERR_PAR_ENUM_REDECLARED, name);
    Ast_DeclareEnumConst(name, Par_EnumValue++);
}

// Build the statement writing one flattened initializer into its object.
Ast_Node *Par_InitStore(Ast_Var *var, int32_t off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, Ast_Line line)
{
    int32_t at_off = bits ? off - bits->am_offset : off;
    Ast_Type *outer = bits ? bits->am_owner : type;

    Ast_Node *addr = Ast_NewUnary(AST_NODE_KIND_CAST, Ast_NewUnary(AST_NODE_KIND_ADDR, Ast_NewVarNode(var, line), line), line);
    addr->an_type = Ast_NewPointer(&Ast_TypeChar);

    Ast_Node *at = Ast_NewUnary(AST_NODE_KIND_CAST, Ast_NewBinary(AST_NODE_KIND_ADD, addr, Ast_NewNum(at_off, line), line), line);
    at->an_type = Ast_NewPointer(outer);

    Ast_Node *slot = Ast_NewUnary(AST_NODE_KIND_DEREF, at, line);
    if (bits) {
        slot = Ast_NewMemberNode(slot, bits->am_name, line);
    }
    Ast_Node *assign = Ast_NewBinary(AST_NODE_KIND_ASSIGN, slot, value, line);
    assign->an_initstore = true;
    return Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, assign, line);
}

// Record one flattened initializer at a byte offset.
Ast_Node *Par_InitAt(int32_t off, Ast_Type *type, Ast_Member *bits, Ast_Node *value, Ast_Line line)
{
    Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_INIT, value, line);
    node->an_offset   = off;
    node->an_type     = type;
    node->an_bitfield = bits;
    return node;
}

// Move a cursor to the subobject a designator names.
void Par_Designate(Ast_Type *type, Ast_Node *desig, int32_t *index, Ast_Member **member, Ast_Line line)
{
    if (desig->an_memname) {
        Err_AssertAt(line, Sem_IsAggregate(type), ERR_PAR_DESIG_NOT_AGGREGATE, desig->an_memname);
        *member = Ast_FindMember(type, desig->an_memname);
        Err_AssertAt(line, *member, ERR_PAR_DESIG_NO_MEMBER, desig->an_memname);
        return;
    }

    Err_AssertAt(line, type->at_kind == AST_TYPE_KIND_ARRAY, ERR_PAR_DESIG_NOT_ARRAY);
    Err_AssertAt(line, desig->an_index >= 0 && (desig->an_index < type->at_len || type == Par_OpenArray), ERR_PAR_DESIG_OUT_OF_RANGE, (long) desig->an_index);
    *index = (int32_t) desig->an_index;
}

// Step a type and offset into the subobject one designator selected.
void Par_Step(Ast_Type **type, int32_t *off, Ast_Node *desig, int32_t index, Ast_Member *member)
{
    if (desig->an_memname) {
        *off += member->am_offset;
        *type = member->am_type;
        return;
    }
    *off += index * (*type)->at_base->at_size;
    *type = (*type)->at_base;
}

// Stretch the array being sized to hold len elements.
void Par_Reach(const Ast_Type *type, int32_t len)
{
    if (type == Par_OpenArray && len > Par_OpenLen) {
        Par_OpenLen = len;
    }
}

// The type an expression already has.
Ast_Type *Par_ExprType(Ast_Node *node)
{
    Ast_Type *type = NULL;

    switch (node->an_kind) {
        case AST_NODE_KIND_VAR: {
            type = node->an_var->av_type;
        } break;
        case AST_NODE_KIND_COMPOUND:
        case AST_NODE_KIND_CAST: {
            type = node->an_type;
        } break;
        case AST_NODE_KIND_ASSIGN: {
            type = Par_ExprType(node->an_lhs);
        } break;
        case AST_NODE_KIND_COMMA: {
            type = Par_ExprType(node->an_rhs);
        } break;
        case AST_NODE_KIND_DEREF: {
            Ast_Type *outer = Par_ExprType(node->an_lhs);
            if (outer && outer->at_kind == AST_TYPE_KIND_FUNC) {
                type = outer;
            } else {
                type = outer ? outer->at_base : NULL;
            }
        } break;
        case AST_NODE_KIND_ADDR: {
            if (node->an_lhs->an_kind == AST_NODE_KIND_FUNCADDR) {
                type = Par_ExprType(node->an_lhs);
            }
        } break;
        case AST_NODE_KIND_MEMBER: {
            Ast_Type *outer = Par_ExprType(node->an_lhs);
            Ast_Member *member = outer ? Ast_FindMember(outer, node->an_memname) : NULL;
            type = member ? member->am_type : NULL;
        } break;
        case AST_NODE_KIND_FUNCADDR: {
            Ast_Func *func = Ast_FindFunction(node->an_funcname);
            type = func ? Ast_NewPointer(func->af_type) : NULL;
        } break;
        case AST_NODE_KIND_CALL: {
            Ast_Type *callee = Par_ExprType(node->an_lhs);
            if (callee && callee->at_kind == AST_TYPE_KIND_PTR) {
                callee = callee->at_base;
            }
            type = callee && callee->at_kind == AST_TYPE_KIND_FUNC ? callee->at_ret : NULL;
        } break;
        default: {
            // empty
        } break;
    }
    return type;
}

// Return whether init is a string literal filling an array of characters.
bool Par_IsStringInit(const Ast_Type *type, const Ast_Node *init)
{
    if (type->at_kind != AST_TYPE_KIND_ARRAY || init->an_kind != AST_NODE_KIND_STR) {
        return false;
    }
    return Ast_IsInteger(type->at_base) && type->at_base->at_kind != AST_TYPE_KIND_BOOL;
}

// Flatten a string literal into the characters of an array.
void Par_FlattenString(Ast_Type *type, int32_t base, Ast_Node *init, Ast_Node **tail, Ast_Line line)
{
    Ast_Type *elem = type->at_base;
    Ast_Str *str = Ast_StringAt(init->an_stridx);
    int32_t width = (int32_t) str->as_width;
    int32_t len = (int32_t) (str->as_len / str->as_width);
    int32_t count = len + 1;

    Err_AssertAt(line, elem->at_size == width, ERR_PAR_INIT_STRING_WIDTH, elem->at_size, width);
    Par_Reach(type, count);
    if (type != Par_OpenArray && count > type->at_len) {
        Err_AssertAt(line, len <= type->at_len, ERR_PAR_INIT_STRING_TOO_LONG, type->at_len);
        count = len;
    }
    for (int32_t i = 0; i < count; i++) {
        Ast_Node *value = Ast_NewNum((int64_t) Par_GetElement(str->as_data + i * width, str->as_width), line);
        (*tail)->an_next = Par_InitAt(base + i * width, elem, NULL, value, line);
        *tail = (*tail)->an_next;
    }
}

// Fill one slot from the cursor.
void Par_FlattenSlot(Ast_Type *type, int32_t base, Ast_Member *bits, Ast_Node **item, Ast_Node **tail, Ast_Line line)
{
    Ast_Node *iter = *item;
    Ast_Node *value = iter->an_lhs;

    if (value->an_kind == AST_NODE_KIND_INITLIST || Par_IsStringInit(type, value)) {
        Par_Flatten(type, base, bits, value, tail, line);
        *item = iter->an_next;
        return;
    }
    if (Sem_IsAggregate(type) && Par_ExprType(value) == type) {
        (*tail)->an_next = Par_InitAt(base, type, bits, value, line);
        *tail = (*tail)->an_next;
        *item = iter->an_next;
        return;
    }
    if (type->at_kind == AST_TYPE_KIND_ARRAY || Sem_IsAggregate(type)) {
        Par_FlattenList(type, base, item, tail, PAR_LIST_UNBRACED, line);
        return;
    }
    (*tail)->an_next = Par_InitAt(base, type, bits, value, line);
    *tail = (*tail)->an_next;
    *item = iter->an_next;
}

// Walk an aggregate's slots from the cursor.
void Par_FlattenList(Ast_Type *type, int32_t base, Ast_Node **item, Ast_Node **tail, Par_List braced, Ast_Line line)
{
    int32_t index = 0;
    Ast_Member *member = type->at_members;

    while (*item) {
        Ast_Node *iter = *item;

        if (iter->an_desig) {
            if (braced == PAR_LIST_UNBRACED) {
                return;
            }
            int32_t off = base;
            Ast_Node *desig = iter->an_desig;
            Ast_Type *slot = type;

            Par_Designate(type, desig, &index, &member, line);
            Par_Step(&slot, &off, desig, index, member);
            Ast_Member *bits = desig->an_memname && member->am_bits ? member : NULL;
            for (Ast_Node *next = desig->an_next; next; next = next->an_next) {
                int32_t at = 0;
                Ast_Member *inner = NULL;
                Par_Designate(slot, next, &at, &inner, line);
                Par_Step(&slot, &off, next, at, inner);
                bits = next->an_memname && inner->am_bits ? inner : NULL;
            }

            iter->an_desig = NULL;
            Par_Flatten(slot, off, bits, iter->an_lhs, tail, line);
            *item = iter->an_next;
            if (desig->an_memname) {
                member = type->at_kind == AST_TYPE_KIND_UNION ? NULL : member->am_next;
            } else {
                Par_Reach(type, ++index);
            }
            continue;
        }

        if (type->at_kind == AST_TYPE_KIND_ARRAY) {
            if (index >= type->at_len && braced == PAR_LIST_UNBRACED) {
                return;
            }
            Err_AssertAt(line, index < type->at_len || type == Par_OpenArray, ERR_PAR_INIT_TOO_MANY_ELEMENTS, type->at_len);
            Par_FlattenSlot(type->at_base, base + index * type->at_base->at_size, NULL, item, tail, line);
            Par_Reach(type, ++index);
            continue;
        }

        if (! member && braced == PAR_LIST_UNBRACED) {
            return;
        }
        Err_AssertAt(line, member, ERR_PAR_INIT_TOO_MANY_MEMBERS, Sem_TypeName(type));
        Par_FlattenSlot(member->am_type, base + member->am_offset, member->am_bits ? member : NULL, item, tail, line);
        member = type->at_kind == AST_TYPE_KIND_UNION ? NULL : member->am_next;
    }
}

// Flatten one initializer, braced or not, into the object at base.
void Par_Flatten(Ast_Type *type, int32_t base, Ast_Member *bits, Ast_Node *init, Ast_Node **tail, Ast_Line line)
{
    if (init->an_kind == AST_NODE_KIND_COMPOUND && init->an_type == type) {
        for (Ast_Node *item = init->an_items; item; item = item->an_next) {
            (*tail)->an_next = Par_InitAt(base + item->an_offset, item->an_type, item->an_bitfield, item->an_lhs, line);
            *tail = (*tail)->an_next;
        }
        return;
    }

    if (Par_IsStringInit(type, init)) {
        Par_FlattenString(type, base, init, tail, line);
        return;
    }
    if (init->an_kind != AST_NODE_KIND_INITLIST) {
        Err_AssertAt(line, type->at_kind != AST_TYPE_KIND_ARRAY, ERR_PAR_INIT_ARRAY_NOT_BRACED);
        (*tail)->an_next = Par_InitAt(base, type, bits, init, line);
        *tail = (*tail)->an_next;
        return;
    }

    Ast_Node *item = init->an_items;
    if (type->at_kind != AST_TYPE_KIND_ARRAY && ! Sem_IsAggregate(type)) {
        Err_AssertAt(line, item, ERR_PAR_INIT_EMPTY);
        Par_Flatten(type, base, bits, item->an_lhs, tail, line);
        return;
    }
    if (item && ! item->an_desig && ! item->an_next && Par_IsStringInit(type, item->an_lhs)) {
        Par_FlattenString(type, base, item->an_lhs, tail, line);
        return;
    }
    Par_FlattenList(type, base, &item, tail, PAR_LIST_BRACED, line);
}

// Flatten an initializer, sizing an array of unknown length from it.
Ast_Node *Par_FlattenInit(Ast_Type **type, Ast_Node *init, Ast_Line line)
{
    Ast_Node head = {0};
    Ast_Node *tail = &head;

    Par_OpenArray = Ast_IsUnsized(*type) ? *type : NULL;
    Par_OpenLen = 0;
    Par_Flatten(*type, 0, NULL, init, &tail, line);
    if (Par_OpenArray) {
        *type = Ast_SizeArray(Par_OpenArray, Par_OpenLen);
        Par_OpenArray = NULL;
    }
    return head.an_next;
}

// Lower a flattened initializer to the statements filling a local.
Ast_Node *Par_InitFlat(Ast_Var *var, Ast_Node *flat, Ast_Line line)
{
    Ast_Node *zero = Ast_NewUnary(AST_NODE_KIND_ZERO, Ast_NewVarNode(var, line), line);
    zero->an_size = var->av_type->at_size;

    Ast_Node *tail = zero;
    for (Ast_Node *item = flat; item; item = item->an_next) {
        tail->an_next = Par_InitStore(var, item->an_offset, item->an_type, item->an_bitfield, item->an_lhs, line);
        tail = tail->an_next;
    }
    return zero;
}

// Lower a local's initializer to the statements that fill it.
Ast_Node *Par_InitLocal(Ast_Var *var, Ast_Node *init, Ast_Line line)
{
    if (init->an_kind != AST_NODE_KIND_INITLIST && var->av_type->at_kind != AST_TYPE_KIND_ARRAY) {
        Ast_Node *assign = Ast_NewBinary(AST_NODE_KIND_ASSIGN, Ast_NewVarNode(var, line), init, line);
        assign->an_initstore = true;
        return Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, assign, line);
    }
    return Par_InitFlat(var, Par_FlattenInit(&var->av_type, init, line), line);
}

// Build the unnamed object a compound literal names.
Ast_Node *Par_CompoundLiteral(Ast_Type *type, Ast_Node *items, Ast_Line line)
{
    Err_AssertAt(line, type->at_complete || Ast_IsUnsized(type), ERR_PAR_LITERAL_NOT_COMPLETE);
    Par_NeedFixedSize(type, line);

    Ast_Node *list = Ast_NewNode(AST_NODE_KIND_INITLIST, line);
    list->an_items = items;

    char *name = Str_Format(".compound.%d", Par_CompoundCount++);
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_COMPOUND, line);
    node->an_items = Par_FlattenInit(&type, list, line);
    node->an_type  = type;

    if (! Par_InFunction) {
        node->an_var = Ast_DeclareGlobal(name, type, line);
        node->an_var->av_storage = AST_STORAGE_STATIC;
        node->an_var->av_init    = node->an_items;
        return node;
    }

    node->an_var = Ast_DeclareVar(name, type, line);
    node->an_body = Par_InitFlat(node->an_var, node->an_items, line);
    return node;
}

// Reject an object whose type has no size.
void Par_CheckComplete(const char *name, Ast_Type *type, Ast_Line line)
{
    Err_AssertAt(line, type->at_complete || Par_DeclStorage == AST_STORAGE_EXTERN, ERR_PAR_OBJECT_NOT_COMPLETE, name);
}

// Reject a variably modified type where only a fixed-size type may go.
void Par_NeedFixedSize(const Ast_Type *type, Ast_Line line)
{
    Err_AssertAt(line, ! Ast_IsVm(type), ERR_PAR_ARRAY_LEN_NOT_CONSTANT);
}

// Build the expression that sizes a type's variable-length arrays.
Ast_Node *Par_SizeExpr(Ast_Type *type, Ast_Line line)
{
    if (type->at_kind != AST_TYPE_KIND_PTR && type->at_kind != AST_TYPE_KIND_ARRAY) {
        return NULL;
    }
    Ast_Node *inner = Par_SizeExpr(type->at_base, line);
    if (! Ast_IsVla(type) || type->at_vsize) {
        return inner;
    }
    Err_AssertAt(line, Par_InFunction, ERR_PAR_VLA_OUTSIDE_FUNCTION);
    Err_AssertAt(line, type->at_vlen->an_kind != AST_NODE_KIND_NOP, ERR_PAR_VLA_STAR_NOT_PROTOTYPE);

    char *name = Str_Format(".vsize.%d", Par_SizeCount++);
    Ast_Node *elem = NULL;

    if (Ast_IsVla(type->at_base)) {
        elem = Ast_NewVarNode(type->at_base->at_vsize, line);
    } else {
        elem = Ast_NewNum(type->at_base->at_size, line);
        elem->an_type = &Ast_TypeULong;
    }
    type->at_vsize = Ast_DeclareVar(name, &Ast_TypeULong, line);
    Str_Free(name);

    Ast_Node *size = Ast_NewBinary(AST_NODE_KIND_VSIZE, type->at_vlen, elem, line);
    size->an_var = type->at_vsize;
    return inner ? Ast_NewBinary(AST_NODE_KIND_COMMA, inner, size, line) : size;
}

// Put the sizing of a type name's variable-length arrays ahead of expr.
Ast_Node *Par_WithSizes(Ast_Type *type, Ast_Node *expr, Ast_Line line)
{
    Ast_Node *size = Par_SizeExpr(type, line);
    return size ? Ast_NewBinary(AST_NODE_KIND_COMMA, size, expr, line) : expr;
}

// Merge a later file-scope declaration's storage class into the first one's.
void Par_Redeclare(Ast_Var *var, Ast_Line line)
{
    bool now = Par_DeclStorage == AST_STORAGE_STATIC;
    bool before = var->av_storage == AST_STORAGE_STATIC;
    bool external = Par_DeclStorage == AST_STORAGE_EXTERN;

    Err_AssertAt(line, external || now == before, ERR_PAR_OBJECT_LINKAGE, var->av_name);
    if (! external && ! now) {
        var->av_storage = AST_STORAGE_NONE;
    }
}

// Declare one file-scope name of the declaration being parsed.
void Par_AddDeclaredType(const char *name, Ast_Type *type, Ast_Node *init, Ast_Line line)
{
    Par_NeedFixedSize(type, line);
    if (Par_DeclStorage == AST_STORAGE_TYPEDEF) {
        Ast_DeclareTypedef(name, type);
        return;
    }
    if (type->at_kind == AST_TYPE_KIND_FUNC) {
        Par_DeclarePrototype(name, type, line);
        return;
    }
    if (! Ast_IsUnsized(type)) {
        Par_CheckComplete(name, type, line);
    }
    Err_AssertAt(line, ! Ast_FindFunction(name), ERR_PAR_FUNCTION_AS_OBJECT, name);

    Ast_Var *var = Ast_FindGlobal(name);
    if (var) {
        Err_AssertAt(line, Ast_IsCompatible(var->av_type, type), ERR_PAR_CONFLICTING_TYPES, name);
        Par_Redeclare(var, line);
    } else {
        var = Ast_DeclareGlobal(name, type, line);
        var->av_storage = Par_DeclStorage;
    }
    if (Ast_IsUnsized(var->av_type)) {
        var->av_type = type;
    }
    if (! init) {
        return;
    }

    Err_AssertAt(line, ! var->av_init, ERR_PAR_OBJECT_REDEFINED, name);
    var->av_init = Par_FlattenInit(&var->av_type, init, line);
    if (var->av_storage == AST_STORAGE_EXTERN) {
        var->av_storage = AST_STORAGE_NONE;
    }
}

// Refuse a second declaration of a name in one block.
void Par_CheckRedeclaration(const char *name, const Ast_Type *type, Ast_Line line)
{
    Ast_Var *var = Ast_FindVarHere(name);
    Ast_Type *def = Ast_FindTypedefHere(name);
    bool externs = var && var->av_storage == AST_STORAGE_EXTERN && Par_DeclStorage == AST_STORAGE_EXTERN;
    bool typedefs = def && ! var && Par_DeclStorage == AST_STORAGE_TYPEDEF;

    if (externs || typedefs) {
        Err_AssertAt(line, Ast_IsCompatible(var ? var->av_type : def, type), ERR_PAR_LOCAL_CONFLICTING_TYPES, name);
        return;
    }
    Err_AssertAt(line, ! var && ! def && ! Ast_IsEnumConstHere(name), ERR_PAR_REDECLARED, name);
}

// Declare a variable inside a function.
Ast_Var *Par_DeclareLocal(const char *name, Ast_Type *type, Ast_Line line)
{
    if (Par_DeclStorage == AST_STORAGE_EXTERN || Ast_IsUnsized(type)) {
        Par_NeedFixedSize(type, line);
    }
    if (Par_DeclStorage == AST_STORAGE_STATIC) {
        Err_AssertAt(line, ! Ast_IsVla(type), ERR_PAR_VLA_STATIC);
    }
    Par_CheckRedeclaration(name, type, line);
    if (Par_DeclStorage == AST_STORAGE_TYPEDEF) {
        Ast_DeclareTypedef(name, type);
        return NULL;
    }
    if (type->at_kind == AST_TYPE_KIND_FUNC) {
        Par_DeclarePrototype(name, type, line);
        return NULL;
    }
    if (! Ast_IsUnsized(type)) {
        Par_CheckComplete(name, type, line);
    }
    if (Par_DeclStorage == AST_STORAGE_EXTERN) {
        return Ast_DeclareExternLocal(name, type, line);
    }
    if (Par_DeclStorage != AST_STORAGE_STATIC) {
        return Ast_DeclareVar(name, type, line);
    }
    char *symbol = Str_Format("%s.%s.%d", Par_CurFuncName, name, Par_StaticCount++);
    Ast_Var *var = Ast_DeclareStaticLocal(name, symbol, type, line);
    var->av_storage = AST_STORAGE_STATIC;
    return var;
}

// Build the statement that gives a declared local its storage or value.
Ast_Node *Par_DefineLocal(Par_Decl *decl, Ast_Var *var, Ast_Node *init, Ast_Line line)
{
    if (var && Ast_IsVla(var->av_type)) {
        Err_AssertAt(line, ! init, ERR_PAR_VLA_INITIALIZED, decl->pc_name);
        Ast_Node *node = Ast_NewUnary(AST_NODE_KIND_VLA, Ast_NewVarNode(var->av_type->at_vsize, line), line);
        node->an_var = var;
        return node;
    }
    if (! init) {
        if (var) {
            Par_CheckComplete(decl->pc_name, var->av_type, line);
        }
        return Ast_NewNode(AST_NODE_KIND_NOP, line);
    }
    Err_AssertAt(line, var, ERR_PAR_TYPEDEF_INITIALIZED);
    Err_AssertAt(line, var->av_storage != AST_STORAGE_EXTERN, ERR_PAR_EXTERN_INITIALIZED, decl->pc_name);
    if (var->av_global) {
        var->av_init = Par_FlattenInit(&var->av_type, init, line);
        return Ast_NewNode(AST_NODE_KIND_NOP, line);
    }
    return Par_InitLocal(var, init, line);
}

// Declare one local and build the statements its declaration becomes.
Ast_Node *Par_AddLocal(Par_Decl *decl, Ast_Node *init, Ast_Line line)
{
    Par_NeedName(decl, line);
    Ast_Type *type = Par_ApplyDecl(Par_DeclType, decl);
    Ast_Var *var = Par_DeclareLocal(decl->pc_name, type, line);
    Ast_Node *size = Par_SizeExpr(type, line);
    Ast_Node *stmt = Par_DefineLocal(decl, var, init, line);

    if (Ast_IsVm(type)) {
        Ast_OpenVmScope();
    }
    if (! size) {
        return stmt;
    }
    Ast_Node *head = Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, size, line);
    head->an_next = stmt;
    return head;
}

// Give each file-scope array still missing its length one element.
void Par_CompleteTentatives(void)
{
    for (Ast_Var *var = Ast_Globals; var; var = var->av_next) {
        if (Ast_IsUnsized(var->av_type) && var->av_storage != AST_STORAGE_EXTERN) {
            Err_WarnAt(var->av_line, ERR_PAR_ARRAY_ASSUMED_ONE, var->av_name);
            var->av_type = Ast_SizeArray(var->av_type, 1);
        }
    }
}

// Append a function to the program.
void Par_AddFunction(Ast_Func *fn, Ast_Line line)
{
    Ast_Func *seen = Ast_FindFunction(fn->af_name);
    if (seen) {
        Ast_Type *type = fn->af_type;
        Err_AssertAt(line, Ast_IsCompatible(seen->af_type, type), ERR_PAR_FUNCTION_CONFLICTING_TYPES, fn->af_name);
        Err_AssertAt(line, ! seen->af_body || ! fn->af_body, ERR_PAR_FUNCTION_REDEFINED, fn->af_name);
        if (fn->af_body) {
            Ast_TypeProto proto = type->at_proto == AST_TYPE_PROTO ? AST_TYPE_PROTO : seen->af_type->at_proto;
            seen->af_body   = fn->af_body;
            seen->af_locals = fn->af_locals;
            seen->af_type   = Ast_NewFunction(seen->af_type->at_ret, type->at_params, type->at_nparams, type->at_va, proto);
        }
        return;
    }

    fn->af_next = NULL;
    if (! Par_ProgHead) {
        Par_ProgHead = Par_ProgTail = fn;
    } else {
        Par_ProgTail->af_next = fn;
        Par_ProgTail = fn;
    }
    Ast_Program = Par_ProgHead;
}

// Record a prototype a declarator spelled out.
void Par_DeclarePrototype(const char *name, Ast_Type *type, Ast_Line line)
{
    Ast_Func *fn = calloc(1, sizeof(Ast_Func));

    Err_AssertAt(line, ! Ast_FindGlobal(name), ERR_PAR_OBJECT_AS_FUNCTION, name);
    fn->af_name   = (char *) name;
    fn->af_type   = type;
    fn->af_static = Par_DeclStorage == AST_STORAGE_STATIC;
    Par_AddFunction(fn, line);
}

// Build the function the parser has just read.
Ast_Func *Par_MakeFunction(Ast_Node *body)
{
    Ast_Func *fn = calloc(1, sizeof(Ast_Func));

    fn->af_name   = Par_CurFuncName;
    fn->af_type   = Par_CurDeclType;
    fn->af_body   = body;
    fn->af_static = Par_CurStatic;
    fn->af_locals = body ? Ast_CurrentLocals() : NULL;
    return fn;
}

// Note the declarator a top-level declaration named.
void Par_BeginExternal(Par_Decl *decl, Ast_Line line)
{
    Par_NeedName(decl, line);
    Ast_Type *type = Par_ApplyDecl(Par_DeclType, decl);

    Par_DeclName    = decl->pc_name;
    Par_DeclLine    = line;
    Par_CurDeclType = type;
    if (type->at_kind != AST_TYPE_KIND_FUNC || Par_DeclStorage == AST_STORAGE_TYPEDEF) {
        Par_InFunction = false;
        return;
    }

    Par_CurStatic   = Par_DeclStorage == AST_STORAGE_STATIC;
    Par_CurFuncName = decl->pc_name;
    Par_InFunction  = true;
    Par_CurFuncVar  = NULL;

    Par_DeclarePrototype(decl->pc_name, type, line);

    Ast_BeginScope();
    for (Ast_Var *param = type->at_params; param; param = param->av_param_next) {
        if (param->av_name) {
            Ast_DeclareParam(param);
        }
    }
}

// Close a top-level declarator that turned out not to be a function definition.
void Par_EndExternal(Ast_Node *init, Ast_Line line)
{
    if (Par_InFunction) {
        Par_AddFunction(Par_MakeFunction(NULL), line);
        Ast_EndScope();
        Par_InFunction = false;
        return;
    }
    Par_AddDeclaredType(Par_DeclName, Par_CurDeclType, init, line);
}

// Close a function definition.
void Par_EndFunction(Ast_Node *body)
{
    if (Par_CurSizes) {
        Ast_Node *stmt = Ast_NewUnary(AST_NODE_KIND_EXPR_STMT, Par_CurSizes, body->an_line);
        stmt->an_next = body->an_body;
        body->an_body = stmt;
    }
    Par_AddFunction(Par_MakeFunction(body), Par_DeclLine);
    Ast_EndScope();
    Par_InFunction = false;
}

// Return the array __func__ names in the function being defined.
Ast_Var *Par_FindFuncName(const char *name, Ast_Line line)
{
    if (! Par_InFunction || ! Str_Equals(name, PAR_FUNC_NAME)) {
        return NULL;
    }
    if (Par_CurFuncVar) {
        return Par_CurFuncVar;
    }

    Ast_Node *str = Ast_NewNode(AST_NODE_KIND_STR, line);
    char *symbol = Str_Format("%s.%s", Par_CurFuncName, PAR_FUNC_NAME);
    Ast_Type *type = Ast_NewUnsizedArray(Ast_Qualify(&Ast_TypeChar, AST_QUAL_CONST));

    str->an_stridx = Ast_AddString(Str_Clone(Par_CurFuncName), strlen(Par_CurFuncName), AST_TYPE_SIZE_CHAR);
    Par_CurFuncVar = Ast_DeclareGlobal(symbol, type, line);
    Par_CurFuncVar->av_storage = AST_STORAGE_STATIC;
    Par_CurFuncVar->av_init = Par_FlattenInit(&Par_CurFuncVar->av_type, str, line);
    return Par_CurFuncVar;
}

// Declare one more top-level name after a comma.
void Par_AddDeclared(Par_Decl *decl, Ast_Node *init, Ast_Line line)
{
    Par_NeedName(decl, line);
    Par_AddDeclaredType(decl->pc_name, Par_ApplyDecl(Par_DeclType, decl), init, line);
}

// Resolve a name used as a value.
Ast_Node *Par_Designator(char *name, Ast_Line line)
{
    Ast_Var *var = Ast_FindVar(name);
    if (! var) {
        var = Par_FindFuncName(name, line);
    }
    if (var) {
        return Ast_NewVarNode(var, line);
    }

    Ast_Func *fn = Ast_FindFunction(name);
    Err_AssertAt(line, fn, ERR_PAR_NAME_NOT_DECLARED, name);
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_FUNCADDR, line);
    node->an_funcname = name;
    return node;
}

// Build a call through its callee.
Ast_Node *Par_MakeCall(Ast_Node *callee, Ast_Node *args, Ast_Line line)
{
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_CALL, line);

    node->an_lhs  = callee;
    node->an_args = args;
    return node;
}

// Build the size a sizeof of a type name yields.
Ast_Node *Par_SizeOfType(Ast_Type *type, Ast_Line line)
{
    Err_AssertAt(line, type->at_complete, ERR_PAR_SIZEOF_NOT_COMPLETE);
    if (! Ast_IsVla(type)) {
        Ast_Node *num = Ast_NewNum(type->at_size, line);
        num->an_type = &Ast_TypeULong;
        return num;
    }
    Ast_Node *size = Par_SizeExpr(type, line);
    Ast_Node *bytes = Ast_NewVarNode(type->at_vsize, line);
    return size ? Ast_NewBinary(AST_NODE_KIND_COMMA, size, bytes, line) : bytes;
}

// Build a jump or a jump's target, noting the variably modified names in scope.
Ast_Node *Par_NewJump(Ast_NodeKind kind, Ast_Line line)
{
    Ast_Node *node = Ast_NewNode(kind, line);
    node->an_vm = Ast_CurrentVmScope();
    return node;
}

// Add one qualifier to an asm statement's set.
Par_AsmQual Par_AddAsmQual(Par_AsmQual quals, Par_AsmQual qual, Ast_Line line)
{
    Err_AssertAt(line, ! (quals & qual), ERR_PAR_ASM_QUALIFIER_DUPLICATE, qual == PAR_ASM_QUAL_VOLATILE ? PAR_ASM_VOLATILE_NAME : PAR_ASM_INLINE_NAME);
    return quals | qual;
}

// Make an asm node from its template.
Ast_Node *Par_NewAsm(Ast_Str text, Ast_Line line)
{
    Err_AssertAt(line, text.as_width == AST_TYPE_SIZE_CHAR, ERR_PAR_ASM_WIDE_STRING);
    Ast_Node *node = Ast_NewNode(AST_NODE_KIND_ASM, line);
    node->an_asm = text.as_data;
    return node;
}
