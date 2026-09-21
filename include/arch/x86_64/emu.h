#ifndef EMU_X86_64_H
#define EMU_X86_64_H

#include <stdint.h>

#include "obj/Elf/load.h"

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

// Registers the SysV ABI and our code generator name, numbered as ModRM does.
#define EMU_X86_64_REG_RAX 0
#define EMU_X86_64_REG_RDX 2
#define EMU_X86_64_REG_RSP 4
#define EMU_X86_64_REG_RDI 7

// Linux syscall numbers the interpreter answers.
#define EMU_X86_64_SYS_WRITE 1
#define EMU_X86_64_SYS_EXIT  60

// Exit status reserved for a fault in the machine rather than in the program.
#define EMU_X86_64_STATUS_FAULT 125

// A running program: the register file, the flags a compare leaves behind, and
// the image the two address.
typedef struct Emu_x86_64_Cpu Emu_x86_64_Cpu;
struct Emu_x86_64_Cpu {
    uint64_t ec_reg[16];
    uint64_t ec_rip;
    int      ec_zf;      // the result was zero
    int      ec_sf;      // the result was negative
    int      ec_of;      // the result overflowed a signed operand
    int      ec_cf;      // the result carried out of an unsigned operand
    int      ec_halted;  // the program asked to stop, or faulted
    int      ec_status;  // the status it stopped with
    const Elf_LoadImage *ec_img;
};

// Running
void Emu_x86_64_Init(Emu_x86_64_Cpu *cpu, const Elf_LoadImage *img);
void Emu_x86_64_Fault(Emu_x86_64_Cpu *cpu, const char *what, uint64_t addr);
uint64_t Emu_x86_64_ReadReg(const Emu_x86_64_Cpu *cpu, int reg, int width);
void Emu_x86_64_WriteReg(Emu_x86_64_Cpu *cpu, int reg, uint64_t value, int width);
uint64_t Emu_x86_64_ReadMem(Emu_x86_64_Cpu *cpu, uint64_t addr, int width);
void Emu_x86_64_WriteMem(Emu_x86_64_Cpu *cpu, uint64_t addr, uint64_t value, int width);
uint64_t Emu_x86_64_RmAddr(Emu_x86_64_Cpu *cpu, const Emu_x86_64_Insn *insn, uint64_t next);
uint64_t Emu_x86_64_ReadRm(Emu_x86_64_Cpu *cpu, const Emu_x86_64_Insn *insn, uint64_t next, int width);
void Emu_x86_64_WriteRm(Emu_x86_64_Cpu *cpu, const Emu_x86_64_Insn *insn, uint64_t next, uint64_t value, int width);
void Emu_x86_64_FlagsSub(Emu_x86_64_Cpu *cpu, uint64_t a, uint64_t b, int width);
void Emu_x86_64_FlagsAdd(Emu_x86_64_Cpu *cpu, uint64_t a, uint64_t b, int width);
void Emu_x86_64_Syscall(Emu_x86_64_Cpu *cpu);
void Emu_x86_64_Step(Emu_x86_64_Cpu *cpu, int trace);
int  Emu_x86_64_Run(const Elf_LoadImage *img, int trace);

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
