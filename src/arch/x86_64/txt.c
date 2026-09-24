// C source file for x86-64 assembly text in AT&T syntax.

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "util/file.h"
#include "util/log.h"
#include "util/str.h"
#include "object/elf.h"
#include "arch/x86_64/asm.h"
#include "arch/x86_64/txt.h"

// 64-bit register names.
static const char *Txt_x86_64_Reg64Name[ASM_X86_64_REG_COUNT] = {
    "rax",
    "rcx",
    "rdx",
    "rbx",
    "rsp",
    "rbp",
    "rsi",
    "rdi",
    "r8",
    "r9",
    "r10",
    "r11",
    "r12",
    "r13",
    "r14",
    "r15"
};

// 8-bit (low-byte) register names.
static const char *Txt_x86_64_Reg8Name[ASM_X86_64_REG_COUNT] = {
    "al",
    "cl",
    "dl",
    "bl",
    "spl",
    "bpl",
    "sil",
    "dil",
    "r8b",
    "r9b",
    "r10b",
    "r11b",
    "r12b",
    "r13b",
    "r14b",
    "r15b"
};

// 32-bit register names.
static const char *Txt_x86_64_Reg32Name[ASM_X86_64_REG_COUNT] = {
    "eax", "ecx", "edx",  "ebx",  "esp",  "ebp",  "esi",  "edi",
    "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d"
};

// 16-bit register names.
static const char *Txt_x86_64_Reg16Name[ASM_X86_64_REG_COUNT] = {
    "ax",  "cx",  "dx",   "bx",   "sp",   "bp",   "si",   "di",
    "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w"
};

// Mnemonics.
static const char *Txt_x86_64_OpName[ASM_X86_64_OP_COUNT] = {
    [ASM_X86_64_OP_MOV]     = "mov",
    [ASM_X86_64_OP_MOVSX]   = "movs",
    [ASM_X86_64_OP_LEA]     = "lea",
    [ASM_X86_64_OP_PUSH]    = "push",
    [ASM_X86_64_OP_POP]     = "pop",
    [ASM_X86_64_OP_ADD]     = "add",
    [ASM_X86_64_OP_SUB]     = "sub",
    [ASM_X86_64_OP_IMUL]    = "imul",
    [ASM_X86_64_OP_IDIV]    = "idiv",
    [ASM_X86_64_OP_DIV]     = "div",
    [ASM_X86_64_OP_AND]     = "and",
    [ASM_X86_64_OP_OR]      = "or",
    [ASM_X86_64_OP_XOR]     = "xor",
    [ASM_X86_64_OP_NOT]     = "not",
    [ASM_X86_64_OP_CALL_REG] = "call",
    [ASM_X86_64_OP_SHL]     = "shl",
    [ASM_X86_64_OP_SAR]     = "sar",
    [ASM_X86_64_OP_SHR]     = "shr",
    [ASM_X86_64_OP_CQO]     = "cqo",
    [ASM_X86_64_OP_NEG]     = "neg",
    [ASM_X86_64_OP_CMP]     = "cmp",
    [ASM_X86_64_OP_SETE]    = "sete",
    [ASM_X86_64_OP_SETNE]   = "setne",
    [ASM_X86_64_OP_SETL]    = "setl",
    [ASM_X86_64_OP_SETLE]   = "setle",
    [ASM_X86_64_OP_SETB]    = "setb",
    [ASM_X86_64_OP_SETBE]   = "setbe",
    [ASM_X86_64_OP_MOVZX]   = "movz",
    [ASM_X86_64_OP_JMP]     = "jmp",
    [ASM_X86_64_OP_JE]      = "je",
    [ASM_X86_64_OP_JNE]     = "jne",
    [ASM_X86_64_OP_CALL]    = "call",
    [ASM_X86_64_OP_RET]     = "ret",
    [ASM_X86_64_OP_SYSCALL] = "syscall"
};

// Write one operand in AT&T syntax.
void Txt_x86_64_Att_WriteOperand(File_Stream *out, const Asm_x86_64_Operand *op)
{
    switch (op->ao_kind) {
        case ASM_X86_64_OPERAND_REG: {
            const char *name = Txt_x86_64_Reg64Name[op->ao_reg];
            if (op->ao_width == ASM_X86_64_WIDTH_8) {
                name = Txt_x86_64_Reg8Name[op->ao_reg];
            } else if (op->ao_width == ASM_X86_64_WIDTH_16) {
                name = Txt_x86_64_Reg16Name[op->ao_reg];
            } else if (op->ao_width == ASM_X86_64_WIDTH_32) {
                name = Txt_x86_64_Reg32Name[op->ao_reg];
            }
            File_Print(out, "%%%s", name);
        } break;
        case ASM_X86_64_OPERAND_IMM: {
            File_Print(out, "$%ld", op->ao_imm);
        } break;
        case ASM_X86_64_OPERAND_MEM: {
            if (op->ao_disp) {
                File_Print(out, "%d(%%%s)", op->ao_disp, Txt_x86_64_Reg64Name[op->ao_reg]);
            } else {
                File_Print(out, "(%%%s)", Txt_x86_64_Reg64Name[op->ao_reg]);
            }
        } break;
        case ASM_X86_64_OPERAND_RIP: {
            File_Print(out, "%s(%%rip)", op->ao_label);
        } break;
        case ASM_X86_64_OPERAND_LABEL: {
            File_Print(out, "%s", op->ao_label);
        } break;
        case ASM_X86_64_OPERAND_NONE:
        case ASM_X86_64_OPERAND_COUNT: {
            // empty
        } break;
    }
}

// Write one instruction: mnemonic plus operands in AT&T order.
void Txt_x86_64_Att_WriteInstr(File_Stream *out, const Asm_x86_64_Item *item)
{
    if (item->ai_op == ASM_X86_64_OP_MOVSX || item->ai_op == ASM_X86_64_OP_MOVZX) {
        const char *stem = item->ai_op == ASM_X86_64_OP_MOVSX ? "movs" : "movz";
        File_Print(out, "  %s%cq", stem, Txt_x86_64_Att_WidthSuffix(item->ai_src.ao_width));
    } else {
        File_Print(out, "  %s", Txt_x86_64_OpName[item->ai_op]);
    }

    bool have_src = item->ai_src.ao_kind != ASM_X86_64_OPERAND_NONE;
    bool have_dst = item->ai_dst.ao_kind != ASM_X86_64_OPERAND_NONE;

    if (have_src) {
        File_PutByte(out, ' ');
        Txt_x86_64_Att_WriteOperand(out, &item->ai_src);
    }
    if (have_dst) {
        File_PutText(out, have_src ? ", " : " ");
        if (item->ai_op == ASM_X86_64_OP_CALL_REG) {
            File_PutByte(out, '*');
        }
        Txt_x86_64_Att_WriteOperand(out, &item->ai_dst);
    }
    File_PutByte(out, '\n');
}

// Walk the instruction list and write AT&T-syntax assembly to out.
void Txt_x86_64_Att_Write(File_Stream *out)
{
    for (Asm_x86_64_Item *item = Asm_x86_64_Items(); item; item = item->ai_next) {
        switch (item->ai_kind) {
            case ASM_X86_64_ITEM_INSTR: {
                Txt_x86_64_Att_WriteInstr(out, item);
            } break;
            case ASM_X86_64_ITEM_LABEL: {
                File_Print(out, "%s:\n", item->ai_label);
            } break;
            case ASM_X86_64_ITEM_GLOBL: {
                File_Print(out, "  .globl %s\n", item->ai_label);
            } break;
            case ASM_X86_64_ITEM_SECTION: {
                if (strcmp(item->ai_secname, ".text") == 0) {
                    File_Print(out, "  .text\n");
                } else {
                    File_Print(out, "  .section %s\n", item->ai_secname);
                }
            } break;
            case ASM_X86_64_ITEM_BYTES: {
                for (size_t i = 0; i < item->ai_nbytes; i++) {
                    File_Print(out, "  .byte %d\n", item->ai_bytes[i]);
                }
            } break;
            case ASM_X86_64_ITEM_ADDR: {
                File_Print(out, "  .quad %s\n", item->ai_label);
            } break;
            case ASM_X86_64_ITEM_DIRECTIVE: {
                File_Print(out, "  %s\n", item->ai_text);
            } break;
            case ASM_X86_64_ITEM_COUNT: {
                // empty
            } break;
        }
    }
}

// Return the AT&T letter naming an operand width.
char Txt_x86_64_Att_WidthSuffix(Asm_x86_64_Width width)
{
    switch (width) {
        case ASM_X86_64_WIDTH_8: {
            return 'b';
        } break;
        case ASM_X86_64_WIDTH_16: {
            return 'w';
        } break;
        case ASM_X86_64_WIDTH_64: {
            return 'q';
        } break;
        default: {
            return 'l';
        }
    }
}

// Return the opcode an extending mnemonic names.
int32_t Txt_x86_64_Att_ExtendOp(const char *mnem, Asm_x86_64_Width *width)
{
    const char *rest = NULL;

    if (strncmp(mnem, "movs", 4) == 0) {
        rest = mnem + 4;
    } else if (strncmp(mnem, "movz", 4) == 0) {
        rest = mnem + 4;
    }
    if (! rest || strlen(rest) != 2 || rest[1] != 'q') {
        return -1;
    }
    switch (rest[0]) {
        case 'b': {
            *width = ASM_X86_64_WIDTH_8;
        } break;
        case 'w': {
            *width = ASM_X86_64_WIDTH_16;
        } break;
        case 'l': {
            *width = ASM_X86_64_WIDTH_32;
        } break;
        default: {
            return -1;
        }
    }
    return mnem[3] == 's' ? ASM_X86_64_OP_MOVSX : ASM_X86_64_OP_MOVZX;
}

// Return the indirect form of an opcode.
int32_t Txt_x86_64_Att_IndirectOp(int32_t opcode)
{
    if (opcode == ASM_X86_64_OP_CALL) {
        return ASM_X86_64_OP_CALL_REG;
    }
    return -1;
}

// True if an opcode branches to a label.
bool Txt_x86_64_Att_IsDirectBranch(int32_t opcode)
{
    return opcode == ASM_X86_64_OP_JMP || opcode == ASM_X86_64_OP_JE || opcode == ASM_X86_64_OP_JNE || opcode == ASM_X86_64_OP_CALL;
}

// Return the register index for an AT&T name like "rax"/"al".
int32_t Txt_x86_64_RegByName(const char *name, Asm_x86_64_Width *width)
{
    for (int32_t i = 0; i < ASM_X86_64_REG_COUNT; i++) {
        if (strcmp(name, Txt_x86_64_Reg64Name[i]) == 0) {
            *width = ASM_X86_64_WIDTH_64;
            return i;
        }
        if (strcmp(name, Txt_x86_64_Reg32Name[i]) == 0) {
            *width = ASM_X86_64_WIDTH_32;
            return i;
        }
        if (strcmp(name, Txt_x86_64_Reg16Name[i]) == 0) {
            *width = ASM_X86_64_WIDTH_16;
            return i;
        }
        if (strcmp(name, Txt_x86_64_Reg8Name[i]) == 0) {
            *width = ASM_X86_64_WIDTH_8;
            return i;
        }
    }
    return -1;
}

// Return the opcode for a mnemonic.
int32_t Txt_x86_64_OpByName(const char *name)
{
    for (int32_t i = 0; i < ASM_X86_64_OP_COUNT; i++) {
        if (Txt_x86_64_OpName[i] && strcmp(name, Txt_x86_64_OpName[i]) == 0) {
            return i;
        }
    }
    return -1;
}

// True if c can open a symbol name.
bool Txt_x86_64_Att_IsNameStart(char c)
{
    return c == '.' || c == '_' || isalpha((uint8_t) c);
}

// True if c can continue a symbol name.
bool Txt_x86_64_Att_IsNameChar(char c)
{
    return c == '.' || c == '_' || c == '$' || isalnum((uint8_t) c);
}

// True if c can open a label.
bool Txt_x86_64_Att_IsLabelStart(char c)
{
    return c == '$' || Txt_x86_64_Att_IsNameStart(c);
}

// True if text names a branch target.
bool Txt_x86_64_Att_IsTarget(const char *text)
{
    const char *p = text;

    while (Txt_x86_64_Att_IsNameChar(*p)) {
        p++;
    }
    return p > text && *p == '\0';
}

// True if text names a symbol a .quad can hold.
bool Txt_x86_64_Att_IsAddress(const char *text)
{
    return Txt_x86_64_Att_IsNameStart(text[0]) && Txt_x86_64_Att_IsTarget(text);
}

// Scan a register name, or return NULL where none stands.
const char *Txt_x86_64_Att_ScanReg(const char *p)
{
    if (! isalpha((uint8_t) *p)) {
        return NULL;
    }
    while (isalnum((uint8_t) *p)) {
        p++;
    }
    return p;
}

// Scan a decimal, octal or hex integer, or return NULL where none stands.
const char *Txt_x86_64_Att_ScanNumber(const char *p, int64_t *out)
{
    const char *start = p;

    if (*p == '-') {
        p++;
    }
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
        if (! isxdigit((uint8_t) *p)) {
            return NULL;
        }
        while (isxdigit((uint8_t) *p)) {
            p++;
        }
    } else {
        if (! isdigit((uint8_t) *p)) {
            return NULL;
        }
        while (isdigit((uint8_t) *p)) {
            p++;
        }
    }
    *out = strtol(start, NULL, 0);
    return p;
}

// Parse one AT&T operand into op.
bool Txt_x86_64_Att_ParseOperand(const char *text, Asm_x86_64_Operand *op)
{
    int64_t disp = 0;
    const char *end;
    const char *p = text;
    Asm_x86_64_Width width;

    if (text[0] == '%') {
        end = Txt_x86_64_Att_ScanReg(text + 1);
        if (end && *end == '\0') {
            int32_t reg = Txt_x86_64_RegByName(text + 1, &width);
            if (reg < 0) {
                return false;
            }
            *op = Asm_x86_64_RegWidth(reg, width);
            return true;
        }
    }
    if (text[0] == '$') {
        end = Txt_x86_64_Att_ScanNumber(text + 1, &disp);
        if (end && *end == '\0') {
            *op = Asm_x86_64_Imm(disp);
            return true;
        }
    }

    while (Txt_x86_64_Att_IsNameChar(*p)) {
        p++;
    }
    if (p > text && strcmp(p, "(%rip)") == 0) {
        *op = Asm_x86_64_Rip(Str_Slice(text, 0, (size_t) (p - text)));
        return true;
    }

    disp = 0;
    p = Txt_x86_64_Att_ScanNumber(text, &disp);
    if (! p) {
        p = text;
    }
    if (p[0] == '(' && p[1] == '%') {
        end = Txt_x86_64_Att_ScanReg(p + 2);
        if (end && end[0] == ')' && end[1] == '\0') {
            char *name = Str_Slice(p, 2, (size_t) (end - p));
            int32_t base = Txt_x86_64_RegByName(name, &width);
            Str_Free(name);
            if (base < 0) {
                return false;
            }
            *op = Asm_x86_64_Mem(base, (int32_t) disp);
            return true;
        }
    }

    if (Txt_x86_64_Att_IsTarget(text)) {
        *op = Asm_x86_64_Target(Str_Clone(text));
        return true;
    }
    return false;
}

// Emit a .byte/.word/.long/.quad list, little-endian or as an address.
void Txt_x86_64_Att_EmitInts(const char *args, size_t width)
{
    Str_List parts = Str_Split(args, ",");
    for (size_t i = 0; i < parts.sl_count; i++) {
        char *text = Str_Trim(parts.sl_items[i]);
        if (! *text) {
            continue;
        }
        if (Txt_x86_64_Att_EmitAddress(text, width)) {
            continue;
        }
        int64_t val = strtol(text, NULL, 0);
        uint8_t bytes[8];
        for (size_t b = 0; b < width; b++) {
            bytes[b] = (val >> (8 * b)) & 0xFF;
        }
        Asm_x86_64_EmitBytes(bytes, width);
    }
    Str_ListFree(&parts);
}

// Emit a `.quad` item that names a symbol.
bool Txt_x86_64_Att_EmitAddress(const char *text, size_t width)
{
    if (! Txt_x86_64_Att_IsNameStart(text[0])) {
        return false;
    }
    if (width != 8 || ! Txt_x86_64_Att_IsAddress(text)) {
        Log_ShowError("as: '%s' is not an address a .quad can hold", text);
    }
    Asm_x86_64_EmitAddress(text);
    return true;
}

// Emit the bytes of a quoted string.
void Txt_x86_64_Att_EmitString(const char *args, Txt_x86_64_Terminate terminate)
{
    const char *p = strchr(args, '"');
    if (! p) {
        return;
    }
    p++;

    size_t len = 0;
    char *buf = Str_Unescape(p, strlen(p), STR_NARROW_WIDTH, &len);

    Asm_x86_64_EmitBytes(buf, len + (terminate == TXT_X86_64_TERMINATED ? 1 : 0));
    Str_Free(buf);
}

// Parse one instruction line ("mnemonic [op[, op]]") into an instruction item.
void Txt_x86_64_Att_ParseInstr(const char *line)
{
    size_t mlen = strcspn(line, " \t");
    if (mlen == 0 || mlen >= 32) {
        Log_ShowError("as: bad mnemonic in '%s'", line);
    }
    char mnem[32];
    memcpy(mnem, line, mlen);
    mnem[mlen] = '\0';

    Asm_x86_64_Width ext_width = ASM_X86_64_WIDTH_NONE;
    int32_t ext_opcode = Txt_x86_64_Att_ExtendOp(mnem, &ext_width);

    int32_t opcode = ext_width ? ext_opcode : Txt_x86_64_OpByName(mnem);
    if (opcode < 0 && mlen >= 2 && strchr("bwlq", mnem[mlen - 1])) {
        mnem[mlen - 1] = '\0';
        opcode = Txt_x86_64_OpByName(mnem);
    }
    if (opcode < 0) {
        Log_ShowError("as: unknown mnemonic '%.*s'", (int) mlen, line);
    }

    Asm_x86_64_Operand ops[2];
    int32_t n_ops = 0;
    const char *rest = line + mlen;
    while (*rest == ' ' || *rest == '\t') {
        rest++;
    }
    if (*rest) {
        Str_List parts = Str_Split(rest, ",");
        for (size_t i = 0; i < parts.sl_count; i++) {
            char *text = Str_Trim(parts.sl_items[i]);
            if (! *text) {
                continue;
            }
            if (n_ops >= 2) {
                Log_ShowError("as: too many operands in '%s'", line);
            }
            const char *op_name = text;
            if (*op_name == '*') {
                opcode = Txt_x86_64_Att_IndirectOp(opcode);
                if (opcode < 0) {
                    Log_ShowError("as: '%s' takes no indirect operand", line);
                }
                op_name++;
            }
            if (! Txt_x86_64_Att_ParseOperand(op_name, &ops[n_ops])) {
                Log_ShowError("as: bad operand '%s'", text);
            }
            n_ops++;
        }
        Str_ListFree(&parts);
    }

    if (Txt_x86_64_Att_IsDirectBranch(opcode)) {
        if (n_ops != 1) {
            Log_ShowError("as: '%s' takes one operand", line);
        }
        if (ops[0].ao_kind != ASM_X86_64_OPERAND_LABEL) {
            Log_ShowError("as: '%s' needs a label", line);
        }
    }

    Asm_x86_64_Item *item = Asm_x86_64_New(ASM_X86_64_ITEM_INSTR);
    item->ai_op = opcode;
    if (n_ops == 2) {
        item->ai_src = ops[0];
        item->ai_dst = ops[1];
        if (ext_width) {
            item->ai_src.ao_width = ext_width;
        }
    } else if (n_ops == 1) {
        item->ai_dst = ops[0];
    }
}

// Parse one directive line, lowering data directives to raw bytes.
void Txt_x86_64_Att_ParseDirective(const char *line)
{
    size_t nlen = strcspn(line, " \t");
    char name[32];
    if (nlen >= sizeof(name)) {
        nlen = sizeof(name) - 1;
    }
    memcpy(name, line, nlen);
    name[nlen] = '\0';

    const char *args = line + nlen;
    while (*args == ' ' || *args == '\t') {
        args++;
    }

    if (Str_Equals(name, ".text")) {
        Asm_x86_64_EmitSection(".text", ELF_SHT_PROGBITS, ELF_SHF_ALLOC | ELF_SHF_EXECINSTR);
    } else if (Str_Equals(name, ".data")) {
        Asm_x86_64_EmitSection(".data", ELF_SHT_PROGBITS, ELF_SHF_ALLOC | ELF_SHF_WRITE);
    } else if (Str_Equals(name, ".rodata")) {
        Asm_x86_64_EmitSection(".rodata", ELF_SHT_PROGBITS, ELF_SHF_ALLOC);
    } else if (Str_Equals(name, ".section")) {
        char *secname = Str_Slice(args, 0, strcspn(args, " ,\t"));
        uint32_t type = ELF_SHT_PROGBITS;
        uint64_t flags;
        const char *quote = strchr(args, '"');
        if (quote) {
            flags = 0;
            for (const char *c = quote + 1; *c && *c != '"'; c++) {
                if (*c == 'a') flags |= ELF_SHF_ALLOC;
                if (*c == 'w') flags |= ELF_SHF_WRITE;
                if (*c == 'x') flags |= ELF_SHF_EXECINSTR;
            }
            const char *at = strchr(args, '@');
            if (at && Str_StartsWith(at + 1, "nobits")) {
                type = ELF_SHT_NOBITS;
            }
        } else if (Str_Equals(secname, ".text")) {
            flags = ELF_SHF_ALLOC | ELF_SHF_EXECINSTR;
        } else if (Str_Equals(secname, ".data")) {
            flags = ELF_SHF_ALLOC | ELF_SHF_WRITE;
        } else {
            flags = ELF_SHF_ALLOC;
        }
        Asm_x86_64_EmitSection(secname, type, flags);
    } else if (Str_Equals(name, ".globl") || Str_Equals(name, ".global")) {
        char *sym = Str_Slice(args, 0, strcspn(args, " ,\t"));
        Asm_x86_64_EmitGlobl("%s", sym);
        Str_Free(sym);
    } else if (Str_Equals(name, ".byte")) {
        Txt_x86_64_Att_EmitInts(args, 1);
    } else if (Str_Equals(name, ".word") || Str_Equals(name, ".short") || Str_Equals(name, ".value")) {
        Txt_x86_64_Att_EmitInts(args, 2);
    } else if (Str_Equals(name, ".long") || Str_Equals(name, ".int")) {
        Txt_x86_64_Att_EmitInts(args, 4);
    } else if (Str_Equals(name, ".quad")) {
        Txt_x86_64_Att_EmitInts(args, 8);
    } else if (Str_Equals(name, ".string") || Str_Equals(name, ".asciz")) {
        Txt_x86_64_Att_EmitString(args, TXT_X86_64_TERMINATED);
    } else if (Str_Equals(name, ".ascii")) {
        Txt_x86_64_Att_EmitString(args, TXT_X86_64_BARE);
    } else if (Str_Equals(name, ".skip") || Str_Equals(name, ".zero") || Str_Equals(name, ".space")) {
        int64_t count = strtol(args, NULL, 0);
        size_t n = count > 0 ? (size_t) count : 0;
        uint8_t *zeros = calloc(n ? n : 1, 1);
        Asm_x86_64_EmitBytes(zeros, n);
        free(zeros);
    } else {
        Asm_x86_64_EmitDirective("%s", line);
    }
}

// Parse one line.
void Txt_x86_64_Att_ParseLine(char *line)
{
    bool inq = false;
    for (char *c = line; *c; c++) {
        if (*c == '"') {
            inq = ! inq;
        } else if (*c == '#' && ! inq) {
            *c = '\0';
            break;
        }
    }

    char *text = Str_Trim(line);
    if (*text == '\0') {
        return;
    }

    char *p = text;
    if (Txt_x86_64_Att_IsLabelStart(*p)) {
        while (Txt_x86_64_Att_IsNameChar(*p)) {
            p++;
        }
    }
    if (p > text && *p == ':') {
        char *name = Str_Slice(text, 0, (size_t) (p - text));
        Asm_x86_64_EmitLabel("%s", name);
        Str_Free(name);
        p++;
        while (*p == ' ' || *p == '\t') {
            p++;
        }
        if (*p) {
            Txt_x86_64_Att_ParseLine(p);
        }
    } else if (text[0] == '.') {
        Txt_x86_64_Att_ParseDirective(text);
    } else {
        Txt_x86_64_Att_ParseInstr(text);
    }
}

// Parse AT&T-syntax assembly text into the instruction list.
void Txt_x86_64_Att_Parse(const char *text)
{
    Asm_x86_64_Reset();
    Str_List lines = Str_Split(text, "\n");
    for (size_t i = 0; i < lines.sl_count; i++) {
        Txt_x86_64_Att_ParseLine(lines.sl_items[i]);
    }
    Str_ListFree(&lines);
}
