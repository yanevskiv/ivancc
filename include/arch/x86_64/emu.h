// C header file for the x86-64 instruction-set emulator.

#ifndef EMU_X86_64_H
#define EMU_X86_64_H

// Standard headers.
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Project headers.
#include "arch/x86_64/enc.h"
#include "arch/x86_64/load.h"

// Bits in a byte, for turning an operand width into a count of bytes.
#define EMU_X86_64_BITS_PER_BYTE 8

// The value a result of each width is truncated to.
#define EMU_X86_64_MASK_8  0xFF
#define EMU_X86_64_MASK_16 0xFFFF
#define EMU_X86_64_MASK_32 0xFFFFFFFF
#define EMU_X86_64_MASK_64 (~(uint64_t) 0)

// A shift count wraps at the operand's width, as the hardware masks it.
#define EMU_X86_64_SHIFT_MASK_32 31
#define EMU_X86_64_SHIFT_MASK_64 63

// Bytes an immediate operand occupies.
#define EMU_X86_64_IMM8  1
#define EMU_X86_64_IMM32 4
#define EMU_X86_64_IMM64 8

// The mask that indexes the register file.
#define EMU_X86_64_REG_INDEX_MASK 15

// The nibble marking a REX prefix, and the bit extending a register.
#define EMU_X86_64_REX_PREFIX_MASK 0xF0
#define EMU_X86_64_REG_HIGH_BIT    8

// The bits of an opcode outside its baked-in register.
#define EMU_X86_64_OPCODE_REG_MASK 0xF8

// Bytes push, pop, call and ret move %rsp by.
#define EMU_X86_64_STACK_SLOT 8

// Memory-mapped device registers, far above anything the linker places.
#define EMU_X86_64_DEV_BASE       0x10000000
#define EMU_X86_64_DEV_DATA_OFF   0  // store: a byte to the terminal
#define EMU_X86_64_DEV_STATUS_OFF 4  // load: nonzero, always ready
#define EMU_X86_64_DEV_HALT_OFF   8  // store: stop with that status
#define EMU_X86_64_DEV_SIZE       16

#define EMU_X86_64_DEV_DATA   (EMU_X86_64_DEV_BASE + EMU_X86_64_DEV_DATA_OFF)
#define EMU_X86_64_DEV_STATUS (EMU_X86_64_DEV_BASE + EMU_X86_64_DEV_STATUS_OFF)
#define EMU_X86_64_DEV_HALT   (EMU_X86_64_DEV_BASE + EMU_X86_64_DEV_HALT_OFF)

// The descriptor the UART writes its bytes to.
#define EMU_X86_64_UART_FD 1

// ei_op2 when an instruction has no second opcode byte.
#define EMU_X86_64_NO_OPCODE2 (-1)

// Linux syscall numbers the interpreter answers.
#define EMU_X86_64_SYS_WRITE 1
#define EMU_X86_64_SYS_EXIT  60

// Exit status reserved for a fault in the machine rather than in the program.
#define EMU_X86_64_STATUS_FAULT 125

// The type a `%llx` conversion takes.
typedef unsigned long long Emu_TypeULLong;

// The signed double-width dividend an idiv consumes.
typedef __int128 Emu_TypeInt128;

// The unsigned double-width dividend a div consumes.
typedef unsigned __int128 Emu_TypeUInt128;

// Registers, numbered as ModRM and REX number them.
typedef enum Emu_x86_64_Reg Emu_x86_64_Reg;
enum Emu_x86_64_Reg {
    EMU_X86_64_REG_RAX,
    EMU_X86_64_REG_RCX,
    EMU_X86_64_REG_RDX,
    EMU_X86_64_REG_RBX,
    EMU_X86_64_REG_RSP,
    EMU_X86_64_REG_RBP,
    EMU_X86_64_REG_RSI,
    EMU_X86_64_REG_RDI,
    EMU_X86_64_REG_R8,
    EMU_X86_64_REG_R9,
    EMU_X86_64_REG_R10,
    EMU_X86_64_REG_R11,
    EMU_X86_64_REG_R12,
    EMU_X86_64_REG_R13,
    EMU_X86_64_REG_R14,
    EMU_X86_64_REG_R15,
    EMU_X86_64_REG_COUNT // number of registers
};

// Register widths a decoded operand can name.
typedef enum Emu_x86_64_OperandWidth Emu_x86_64_OperandWidth;
enum Emu_x86_64_OperandWidth {
    EMU_X86_64_WIDTH_8  = 8,
    EMU_X86_64_WIDTH_16 = 16,
    EMU_X86_64_WIDTH_32 = 32,
    EMU_X86_64_WIDTH_64 = 64
};

// Whether the interpreter writes each instruction to stderr as it runs.
typedef enum Emu_x86_64_Trace Emu_x86_64_Trace;
enum Emu_x86_64_Trace {
    EMU_X86_64_QUIET,
    EMU_X86_64_TRACE
};

// How an instruction reaches its r/m operand.
typedef enum Emu_x86_64_RmKind Emu_x86_64_RmKind;
enum Emu_x86_64_RmKind {
    EMU_X86_64_RM_NONE, // the instruction has no ModRM byte
    EMU_X86_64_RM_REG,  // %reg
    EMU_X86_64_RM_MEM,  // disp(%base)
    EMU_X86_64_RM_RIP,  // disp(%rip)
    EMU_X86_64_RM_COUNT // number of kinds
};

// One decoded instruction, as the fields an interpreter needs.
typedef struct Emu_x86_64_Insn Emu_x86_64_Insn;
struct Emu_x86_64_Insn {
    size_t            ei_len;      // bytes the instruction occupies
    int32_t           ei_op;       // primary opcode byte
    int32_t           ei_op2;      // byte following 0x0F, or -1
    bool              ei_rexw;     // true when REX.W selects a 64-bit operand
    bool              ei_opsize16; // true when a 0x66 prefix selects a 16-bit operand
    Emu_x86_64_Reg    ei_reg;      // ModRM reg field, extended by REX.R
    Emu_x86_64_Reg    ei_rm;       // r/m register, or the base register of a memory operand
    Emu_x86_64_RmKind ei_rmkind;
    int32_t           ei_disp;     // displacement of a MEM or RIP operand
    int64_t           ei_imm;      // immediate or branch displacement, sign-extended
};

// A running program: the register file, the flags and the image they address.
typedef struct Emu_x86_64_Cpu Emu_x86_64_Cpu;
struct Emu_x86_64_Cpu {
    uint64_t ec_reg[EMU_X86_64_REG_COUNT];
    uint64_t ec_rip;
    bool     ec_zf;      // the result was zero
    bool     ec_sf;      // the result was negative
    bool     ec_of;      // the result overflowed a signed operand
    bool     ec_cf;      // the result carried out of an unsigned operand
    bool     ec_halted;  // the program asked to stop, or faulted
    int32_t  ec_status;  // the status it stopped with
    const Load_x86_64_Image *ec_img;
};

// Running
void Emu_x86_64_Init(Emu_x86_64_Cpu *cpu, const Load_x86_64_Image *img);
void Emu_x86_64_Fault(Emu_x86_64_Cpu *cpu, const char *what, uint64_t addr);
uint64_t Emu_x86_64_ReadReg(const Emu_x86_64_Cpu *cpu, Emu_x86_64_Reg reg, Emu_x86_64_OperandWidth width);
void Emu_x86_64_WriteReg(Emu_x86_64_Cpu *cpu, Emu_x86_64_Reg reg, uint64_t value, Emu_x86_64_OperandWidth width);
bool Emu_x86_64_IsDevice(uint64_t addr);
uint64_t Emu_x86_64_ReadDev(Emu_x86_64_Cpu *cpu, uint64_t addr);
void Emu_x86_64_WriteDev(Emu_x86_64_Cpu *cpu, uint64_t addr, uint64_t value);
uint64_t Emu_x86_64_ReadMem(Emu_x86_64_Cpu *cpu, uint64_t addr, Emu_x86_64_OperandWidth width);
void Emu_x86_64_WriteMem(Emu_x86_64_Cpu *cpu, uint64_t addr, uint64_t value, Emu_x86_64_OperandWidth width);
uint64_t Emu_x86_64_RmAddr(Emu_x86_64_Cpu *cpu, const Emu_x86_64_Insn *insn, uint64_t next);
uint64_t Emu_x86_64_ReadRm(Emu_x86_64_Cpu *cpu, const Emu_x86_64_Insn *insn, uint64_t next, Emu_x86_64_OperandWidth width);
void Emu_x86_64_WriteRm(Emu_x86_64_Cpu *cpu, const Emu_x86_64_Insn *insn, uint64_t next, uint64_t value, Emu_x86_64_OperandWidth width);
void Emu_x86_64_FlagsSub(Emu_x86_64_Cpu *cpu, uint64_t a, uint64_t b, Emu_x86_64_OperandWidth width);
void Emu_x86_64_FlagsAdd(Emu_x86_64_Cpu *cpu, uint64_t a, uint64_t b, Emu_x86_64_OperandWidth width);
void Emu_x86_64_Syscall(Emu_x86_64_Cpu *cpu);
void Emu_x86_64_Step(Emu_x86_64_Cpu *cpu, Emu_x86_64_Trace trace);
int32_t Emu_x86_64_Run(const Load_x86_64_Image *img, Emu_x86_64_Trace trace);

// Decoding
int64_t Emu_x86_64_ReadImm(const uint8_t *p, size_t n);
bool Emu_x86_64_HasModRM(int32_t op);
bool Emu_x86_64_HasModRM2(int32_t op2);
size_t Emu_x86_64_DecodeModRM(const uint8_t *p, size_t avail, uint8_t rex, Emu_x86_64_Insn *insn);
Emu_x86_64_OperandWidth Emu_x86_64_Width(const Emu_x86_64_Insn *insn);
size_t Emu_x86_64_Decode(const uint8_t *code, size_t avail, Emu_x86_64_Insn *insn);

// Naming what was decoded
const char *Emu_x86_64_RegName(Emu_x86_64_Reg reg, Emu_x86_64_OperandWidth width);
const char *Emu_x86_64_Mnemonic(const Emu_x86_64_Insn *insn);
void        Emu_x86_64_FormatRm(const Emu_x86_64_Insn *insn, Emu_x86_64_OperandWidth width, uint64_t next, char *out, size_t n);
void        Emu_x86_64_Format(const Emu_x86_64_Insn *insn, uint64_t rip, char *out, size_t n);

#endif // EMU_X86_64_H
