/*
 * C header file for the ivanemu emulator.
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

#ifndef EMU_H
#define EMU_H

// Standard headers.
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

// Project headers.
#include "util/console/err.h"
#include "util/console/log.h"
#include "util/object/elf.h"
#include "util/object/load.h"
#include "util/str.h"
#include "arch/x86_64/cpu.h"

// Target architecture selected when no -march= is given.
#define EMU_DEFAULT_ARCH "x86_64"

// Machine-option prefix recognised inside -m (e.g. -march=x86_64).
#define EMU_MARCH_PREFIX "arch="

// The status a shell reports for a death by signal, less the signal.
#define EMU_SIGNAL_STATUS_BASE 128

// Memory-mapped device registers, far above anything the linker places.
#define EMU_DEV_BASE       0x10000000
#define EMU_DEV_DATA_OFF   0  // store: a byte to the terminal
#define EMU_DEV_STATUS_OFF 4  // load: nonzero, always ready
#define EMU_DEV_HALT_OFF   8  // store: stop with that status
#define EMU_DEV_SIZE       16

#define EMU_DEV_DATA   (EMU_DEV_BASE + EMU_DEV_DATA_OFF)
#define EMU_DEV_STATUS (EMU_DEV_BASE + EMU_DEV_STATUS_OFF)
#define EMU_DEV_HALT   (EMU_DEV_BASE + EMU_DEV_HALT_OFF)

// The descriptor the UART writes its bytes to.
#define EMU_UART_FD 1

// Linux syscall numbers the emulator answers.
#define EMU_SYS_WRITE          1
#define EMU_SYS_BRK            12
#define EMU_SYS_RT_SIGACTION   13
#define EMU_SYS_RT_SIGPROCMASK 14
#define EMU_SYS_RT_SIGRETURN   15
#define EMU_SYS_GETPID         39
#define EMU_SYS_EXIT           60
#define EMU_SYS_KILL           62
#define EMU_SYS_CLOCK_GETTIME  228
#define EMU_SYS_EXIT_GROUP     231

// The layout of Linux's struct timespec.
#define EMU_TIMESPEC_SEC_OFF  0
#define EMU_TIMESPEC_NSEC_OFF 8
#define EMU_TIMESPEC_SIZE     16

// Linux errno values a failed syscall returns negated in %rax.
#define EMU_ERRNO_FAULT 14
#define EMU_ERRNO_INVAL 22
#define EMU_ERRNO_NOSYS 38

// The signal numbers kill checks its argument against.
#define EMU_SIGNAL_NONE   0
#define EMU_SIGNAL_FIRST  1
#define EMU_SIGNAL_RT_MIN 32
#define EMU_SIGNAL_MAX    64

// The bytes of Linux's sigset_t.
#define EMU_SIGSET_SIZE 8

// rt_sigprocmask's ways of changing the mask.
#define EMU_SIG_BLOCK   0
#define EMU_SIG_UNBLOCK 1
#define EMU_SIG_SETMASK 2

// The layout of Linux's struct sigaction for rt_sigaction.
#define EMU_SIGACTION_HANDLER_OFF  0
#define EMU_SIGACTION_FLAGS_OFF    8
#define EMU_SIGACTION_RESTORER_OFF 16
#define EMU_SIGACTION_MASK_OFF     24
#define EMU_SIGACTION_SIZE         32

// The handlers rt_sigaction takes beside a function's address.
#define EMU_SIG_DFL 0
#define EMU_SIG_IGN 1

// The sa_flags Linux keeps.
#define EMU_SA_NOCLDSTOP      0x1
#define EMU_SA_NOCLDWAIT      0x2
#define EMU_SA_SIGINFO        0x4
#define EMU_SA_EXPOSE_TAGBITS 0x800
#define EMU_SA_RESTORER       0x04000000
#define EMU_SA_ONSTACK        0x08000000
#define EMU_SA_RESTART        0x10000000
#define EMU_SA_NODEFER        0x40000000
#define EMU_SA_RESETHAND      0x80000000
#define EMU_SA_KNOWN          (EMU_SA_NOCLDSTOP | EMU_SA_NOCLDWAIT | EMU_SA_SIGINFO | EMU_SA_EXPOSE_TAGBITS | EMU_SA_RESTORER | EMU_SA_ONSTACK | EMU_SA_RESTART | EMU_SA_NODEFER | EMU_SA_RESETHAND)

// The si_code values of the signals the emulator sends.
#define EMU_SI_USER     0
#define EMU_SI_KERNEL   0x80
#define EMU_ILL_ILLOPN  2
#define EMU_FPE_INTDIV  1
#define EMU_SEGV_MAPERR 1

// The layout of Linux's rt_sigframe.
#define EMU_FRAME_PRETCODE_OFF 0
#define EMU_FRAME_UC_OFF       8
#define EMU_FRAME_UC_FLAGS_OFF 8
#define EMU_FRAME_SC_OFF       48
#define EMU_FRAME_SIGMASK_OFF  304
#define EMU_FRAME_INFO_OFF     312
#define EMU_FRAME_SIZE         440

// The layout of Linux's struct sigcontext after its general registers.
#define EMU_SC_RIP_OFF     128
#define EMU_SC_EFLAGS_OFF  136
#define EMU_SC_CS_OFF      144
#define EMU_SC_SS_OFF      150
#define EMU_SC_ERR_OFF     152
#define EMU_SC_TRAPNO_OFF  160
#define EMU_SC_OLDMASK_OFF 168
#define EMU_SC_CR2_OFF     176
#define EMU_SC_FPSTATE_OFF 184

// The layout of Linux's siginfo.
#define EMU_INFO_SIGNO_OFF 0
#define EMU_INFO_CODE_OFF  8
#define EMU_INFO_PID_OFF   16
#define EMU_INFO_UID_OFF   20
#define EMU_INFO_ADDR_OFF  16

// The uc_flags Linux gives a frame with no XSAVE state.
#define EMU_UC_SIGCONTEXT_SS     0x2
#define EMU_UC_STRICT_RESTORE_SS 0x4

// The selectors of a 64-bit user program's code and stack.
#define EMU_USER_CS 0x33
#define EMU_USER_SS 0x2b

// The bits of %rflags a frame holds.
#define EMU_RFLAGS_CF    0x1
#define EMU_RFLAGS_FIXED 0x2
#define EMU_RFLAGS_PF    0x4
#define EMU_RFLAGS_ZF    0x40
#define EMU_RFLAGS_SF    0x80
#define EMU_RFLAGS_IF    0x200
#define EMU_RFLAGS_OF    0x800

// The red zone a frame leaves under the interrupted %rsp.
#define EMU_FRAME_REDZONE 128

// The alignments of a frame and of its fpstate.
#define EMU_FRAME_ALIGN         16
#define EMU_FRAME_FPSTATE_ALIGN 64

// The layout of the FXSAVE area a frame's fpstate points to.
#define EMU_FXSAVE_FCW_OFF        0
#define EMU_FXSAVE_FSW_OFF        2
#define EMU_FXSAVE_MXCSR_OFF      24
#define EMU_FXSAVE_MXCSR_MASK_OFF 28
#define EMU_FXSAVE_ST_OFF         32
#define EMU_FXSAVE_XMM_OFF        160
#define EMU_FXSAVE_REG_SIZE       16
#define EMU_FXSAVE_SIZE           512

// The x87 and SSE control words a handler starts with.
#define EMU_FCW_DEFAULT   0x037F
#define EMU_MXCSR_DEFAULT 0x1F80
#define EMU_MXCSR_MASK    0xFFFF

// The place of the x87 stack's top in its status word.
#define EMU_FSW_TOP_SHIFT 11

// Whether the emulator writes each instruction to stderr before it runs.
typedef enum Emu_Trace Emu_Trace;
enum Emu_Trace {
    EMU_QUIET,
    EMU_TRACE
};

// What sent a pending signal.
typedef enum Emu_Source Emu_Source;
enum Emu_Source {
    EMU_SOURCE_KILL,   // the program's kill of itself
    EMU_SOURCE_FAULT,  // an exception of the CPU's
    EMU_SOURCE_KERNEL, // a frame the emulator could not build or restore
    EMU_SOURCE_COUNT   // number of sources
};

// A signal's disposition as rt_sigaction sets it.
typedef struct Emu_Action Emu_Action;
struct Emu_Action {
    uint64_t ea_handler; // the handler's address, EMU_SIG_DFL or EMU_SIG_IGN
    uint64_t ea_flags;
    uint64_t ea_restorer;
    uint64_t ea_mask;    // the signals blocked while the handler runs
};

// What a pending signal's siginfo says.
typedef struct Emu_Info Emu_Info;
struct Emu_Info {
    Emu_Source ei_source;
    int32_t    ei_code;  // si_code
    uint64_t   ei_addr;  // the address a fault names
};

// A running program: its image, its signals and how it stopped.
typedef struct Emu_Guest Emu_Guest;
struct Emu_Guest {
    Load_Image       *eg_img;
    bool              eg_halted;                     // the program exited or a signal ended it
    int32_t           eg_status;                     // the status it exited with
    int32_t           eg_signal;                     // the signal that ended it, or 0
    Emu_Action        eg_action[EMU_SIGNAL_MAX + 1];
    Emu_Info          eg_info[EMU_SIGNAL_MAX + 1];   // what sent each pending signal
    uint32_t          eg_queued[EMU_SIGNAL_MAX + 1]; // the instances of each signal pending
    uint64_t          eg_pending;                    // the signals with an instance pending
    uint64_t          eg_blocked;                    // the signals the program blocks
    uint64_t          eg_trapno;                     // the vector of the last exception
    uint64_t          eg_err;                        // the error code of the last exception
};

// The host's environment.
extern char **environ;

// Usage
void Emu_Usage(const char *prog);

// Inspection
void Emu_ShowImage(const Load_Image *img);
void Emu_Disassemble(const Load_Image *img);

// Bus
uint8_t *Emu_MapMemory(void *ctx, uint64_t addr, size_t *avail);
uint64_t Emu_LoadDevice(void *ctx, uint64_t addr, size_t size);
void     Emu_StoreDevice(void *ctx, uint64_t addr, size_t size, uint64_t value);

// Memory
void     Emu_Put(uint8_t *ptr, uint64_t value, size_t size);
uint64_t Emu_Get(const uint8_t *ptr, size_t size);

// Signals
uint64_t Emu_SignalBit(int32_t sig);
bool     Emu_IsDefaultIgnored(int32_t sig);
bool     Emu_IsDefaultStop(int32_t sig);
bool     Emu_IsSynchronous(int32_t sig);
uint64_t Emu_Unblockable(void);
bool     Emu_IsIgnored(const Emu_Guest *guest, int32_t sig);
void     Emu_Send(Emu_Guest *guest, int32_t sig, Emu_Info info);
void     Emu_Force(Emu_Guest *guest, int32_t sig, Emu_Info info);
void     Emu_Discard(Emu_Guest *guest, int32_t sig);
void     Emu_Inherit(Emu_Guest *guest);
uint64_t Emu_Kill(Emu_Guest *guest, int32_t pid, int32_t sig);
uint64_t Emu_SigAction(Emu_Guest *guest, int32_t sig, uint64_t act, uint64_t oact, uint64_t size);
uint64_t Emu_SigProcMask(Emu_Guest *guest, int32_t how, uint64_t set, uint64_t oset, uint64_t size);

// Registers
uint64_t Emu_ReadFlags(const Cpu_x86_64_State *cpu);
void     Emu_WriteFlags(Cpu_x86_64_State *cpu, uint64_t flags);
void     Emu_SaveFpu(Cpu_x86_64_State *cpu, uint8_t *area);
void     Emu_RestoreFpu(Cpu_x86_64_State *cpu, const uint8_t *area);
void     Emu_ResetFpu(Cpu_x86_64_State *cpu);

// Delivery
bool     Emu_PushFrame(Emu_Guest *guest, Cpu_x86_64_State *cpu, int32_t sig, const Emu_Action *action, const Emu_Info *info);
uint64_t Emu_SigReturn(Emu_Guest *guest, Cpu_x86_64_State *cpu);
int32_t  Emu_NextSignal(const Emu_Guest *guest);
void     Emu_Deliver(Emu_Guest *guest, Cpu_x86_64_State *cpu);
void     Emu_Fault(Emu_Guest *guest, Cpu_x86_64_State *cpu, uint64_t rip);

// Syscalls
uint64_t Emu_Brk(Emu_Guest *guest, uint64_t addr);
uint64_t Emu_ClockGettime(Emu_Guest *guest, int32_t clock, uint64_t addr);
void     Emu_Syscall(Emu_Guest *guest, Cpu_x86_64_State *cpu);

// Running
void    Emu_ShowStep(Emu_Guest *guest, uint64_t rip);
int32_t Emu_Run(Load_Image *img, Emu_Trace trace, int32_t *sig);
void    Emu_Raise(int32_t sig);

#endif // EMU_H
