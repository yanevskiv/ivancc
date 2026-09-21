#ifndef EMU_X86_64_H
#define EMU_X86_64_H

#include <stdint.h>

// Register widths a decoded operand can name, in bits.
#define EMU_X86_64_WIDTH_8  8
#define EMU_X86_64_WIDTH_32 32
#define EMU_X86_64_WIDTH_64 64

// How an instruction reaches its r/m operand.
typedef enum Emu_x86_64_RmKind Emu_x86_64_RmKind;
enum Emu_x86_64_RmKind {
    EMU_X86_64_RM_NONE, // the instruction has no ModRM byte
    EMU_X86_64_RM_REG,  // %reg
    EMU_X86_64_RM_MEM,  // disp(%base)
    EMU_X86_64_RM_RIP   // disp(%rip)
};

// One decoded instruction, as the fields an interpreter needs rather than as
// the bytes it came from.
typedef struct Emu_x86_64_Insn Emu_x86_64_Insn;
struct Emu_x86_64_Insn {
    int     ei_len;     // bytes the instruction occupies
    int     ei_op;      // primary opcode byte
    int     ei_op2;     // byte following 0x0F, or -1 when there is none
    int     ei_rexw;    // true when REX.W selects a 64-bit operand
    int     ei_reg;     // ModRM reg field, extended by REX.R
    int     ei_rm;      // r/m register, or the base register of a memory operand
    Emu_x86_64_RmKind ei_rmkind;
    int32_t ei_disp;    // displacement of a MEM or RIP operand
    int64_t ei_imm;     // immediate or branch displacement, sign-extended
};

// Decoding
int64_t Emu_x86_64_ReadImm(const uint8_t *p, int n);
int Emu_x86_64_HasModRM(int op);
int Emu_x86_64_HasModRM2(int op2);
int Emu_x86_64_DecodeModRM(const uint8_t *p, int avail, int rex, Emu_x86_64_Insn *insn);
int Emu_x86_64_Decode(const uint8_t *code, int avail, Emu_x86_64_Insn *insn);

// Naming what was decoded
const char *Emu_x86_64_RegName(int reg, int width);
const char *Emu_x86_64_Mnemonic(const Emu_x86_64_Insn *insn);
void        Emu_x86_64_FormatRm(const Emu_x86_64_Insn *insn, int width, uint64_t next, char *out, int n);
void        Emu_x86_64_Format(const Emu_x86_64_Insn *insn, uint64_t rip, char *out, int n);

#endif // EMU_X86_64_H
