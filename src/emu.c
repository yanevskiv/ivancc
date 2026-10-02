/*
 * C source file for the ivanemu emulator.
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

// Standard headers.
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
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
#define EMU_SYS_WRITE 1
#define EMU_SYS_EXIT  60

// Linux errno values a failed syscall returns negated in %rax.
#define EMU_ERRNO_FAULT 14
#define EMU_ERRNO_NOSYS 38

// Whether the emulator writes each instruction to stderr before it runs.
typedef enum Emu_Trace Emu_Trace;
enum Emu_Trace {
    EMU_QUIET,
    EMU_TRACE
};

// A running program: its image, and how it stopped.
typedef struct Emu_Guest Emu_Guest;
struct Emu_Guest {
    const Load_Image *eg_img;
    bool                     eg_halted; // the program asked to stop, or faulted
    int32_t                  eg_status; // the status it stopped with
    int32_t                  eg_signal; // the signal a fault ended it with, or 0
};

// The host's environment.
extern char **environ;

// Show usage information and exit.
static void Emu_Usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s [options] PROGRAM [ARGUMENT...]\n"
        "  -d          disassemble instead of running\n"
        "  -i          print what the loader made of the file and stop\n"
        "  -t          trace each instruction to stderr as it runs\n"
        "  -march=ARCH target architecture (default: " EMU_DEFAULT_ARCH ")\n",
        prog);
    exit(1);
}

// Print what the loader made of an executable.
static void Emu_ShowImage(const Load_Image *img)
{
    fprintf(stdout, "entry  0x%llx\n", (Cpu_x86_64_TypeULLong) img->li_entry);
    fprintf(stdout, "base   0x%llx\n", (Cpu_x86_64_TypeULLong) img->li_base);
    fprintf(stdout, "size   0x%llx\n", (Cpu_x86_64_TypeULLong) img->li_size);
    fprintf(stdout, "stack  0x%llx\n", (Cpu_x86_64_TypeULLong) img->li_stack);
}

// Disassemble forward from the image's base until the bytes stop decoding.
static void Emu_Disassemble(const Load_Image *img)
{
    uint64_t rip = img->li_base;
    for (;;) {
        size_t avail = img->li_base + img->li_size - rip;
        const uint8_t *code = Load_At(img, rip, 1);
        Cpu_x86_64_Insn insn;
        char text[128];

        if (! code || ! Cpu_x86_64_Decode(code, avail, &insn)) {
            fprintf(stdout, "%016llx: (bad)\n", (Cpu_x86_64_TypeULLong) rip);
            return;
        }
        Cpu_x86_64_Format(&insn, rip, text, sizeof(text));
        fprintf(stdout, "%016llx: %s\n", (Cpu_x86_64_TypeULLong) rip, text);
        rip += insn.ci_len;
    }
}

// Return the image's memory at addr and the bytes left from there, or NULL.
static uint8_t *Emu_MapMemory(void *ctx, uint64_t addr, size_t *avail)
{
    const Emu_Guest *guest = ctx;
    uint64_t off = addr - guest->eg_img->li_base;
    if (off >= guest->eg_img->li_size) {
        return NULL;
    }
    *avail = guest->eg_img->li_size - off;
    return guest->eg_img->li_mem + off;
}

// Read a device register, the UART being write-only and always ready.
static uint64_t Emu_LoadDevice(void *ctx, uint64_t addr, size_t size)
{
    (void) ctx;
    (void) size;
    return addr == EMU_DEV_STATUS ? 1 : 0;
}

// Write a device register.
static void Emu_StoreDevice(void *ctx, uint64_t addr, size_t size, uint64_t value)
{
    Emu_Guest *guest = ctx;

    (void) size;
    switch (addr) {
        case EMU_DEV_DATA: {
            uint8_t byte = value & CPU_X86_64_MASK_8;
            write(EMU_UART_FD, &byte, sizeof(byte));
        } break;
        case EMU_DEV_HALT: {
            guest->eg_halted = true;
            guest->eg_status = value & CPU_X86_64_MASK_8;
        } break;
        default: {
            // empty
        } break;
    }
}

// Answer a syscall as Linux does and fail the ones it lacks with ENOSYS.
static void Emu_Syscall(Emu_Guest *guest, Cpu_x86_64_State *cpu)
{
    uint64_t *rax = &cpu->cs_reg[CPU_X86_64_REG_RAX];
    switch (*rax) {
        case EMU_SYS_WRITE: {
            uint64_t fd = cpu->cs_reg[CPU_X86_64_REG_RDI];
            uint64_t buf = cpu->cs_reg[CPU_X86_64_REG_RSI];
            uint64_t len = cpu->cs_reg[CPU_X86_64_REG_RDX];
            const uint8_t *p = Load_At(guest->eg_img, buf, len);
            if (! p && len) {
                *rax = -(uint64_t) EMU_ERRNO_FAULT;
            } else {
                ssize_t n = write((int) fd, p, (size_t) len);
                *rax = n < 0 ? -(uint64_t) errno : (uint64_t) n;
            }
        } break;
        case EMU_SYS_EXIT: {
            guest->eg_halted = true;
            guest->eg_status = cpu->cs_reg[CPU_X86_64_REG_RDI] & CPU_X86_64_MASK_8;
        } break;
        default: {
            *rax = -(uint64_t) EMU_ERRNO_NOSYS;
        }
    }
}

// Return the signal Linux ends a program with for an exception.
static int32_t Emu_Signal(Cpu_x86_64_Vector vector)
{
    switch (vector) {
        case CPU_X86_64_VECTOR_DE: {
            return SIGFPE;
        } break;
        case CPU_X86_64_VECTOR_UD: {
            return SIGILL;
        } break;
        default: {
            return SIGSEGV;
        }
    }
}

// Write the instruction at rip to stderr before it runs, if it decodes.
static void Emu_ShowStep(Emu_Guest *guest, uint64_t rip)
{
    size_t avail = 0;
    const uint8_t *code = Emu_MapMemory(guest, rip, &avail);
    Cpu_x86_64_Insn insn;
    char text[128];

    if (! code || ! Cpu_x86_64_Decode(code, avail, &insn)) {
        return;
    }
    Cpu_x86_64_Format(&insn, rip, text, sizeof(text));
    fprintf(stderr, "%016llx: %s\n", (Cpu_x86_64_TypeULLong) rip, text);
}

// Run a loaded program to completion and return its status and fault signal.
static int32_t Emu_Run(const Load_Image *img, Emu_Trace trace, int32_t *sig)
{
    Emu_Guest guest = {
        .eg_img = img
    };
    Cpu_x86_64_Bus bus = {
        .cb_ctx     = &guest,
        .cb_map     = Emu_MapMemory,
        .cb_io_base = EMU_DEV_BASE,
        .cb_io_size = EMU_DEV_SIZE,
        .cb_load    = Emu_LoadDevice,
        .cb_store   = Emu_StoreDevice
    };
    Cpu_x86_64_State cpu;

    Cpu_x86_64_Init(&cpu, &bus, img->li_entry, img->li_stack);
    while (! guest.eg_halted) {
        if (trace == EMU_TRACE) {
            Emu_ShowStep(&guest, cpu.cs_rip);
        }
        Cpu_x86_64_Step(&cpu);
        switch (cpu.cs_trap) {
            case CPU_X86_64_TRAP_SYSCALL: {
                Emu_Syscall(&guest, &cpu);
            } break;
            case CPU_X86_64_TRAP_EXCEPTION: {
                guest.eg_halted = true;
                guest.eg_signal = Emu_Signal(cpu.cs_vector);
            } break;
            default: {
                // empty
            } break;
        }
    }
    *sig = guest.eg_signal;
    return guest.eg_status;
}

// Die by the signal a program faulted with as the program would have died.
static void Emu_Raise(int32_t sig)
{
    struct sigaction act = {0};
    struct rlimit core = {0};
    sigset_t set;

    act.sa_handler = SIG_DFL;
    sigaction(sig, &act, NULL);
    setrlimit(RLIMIT_CORE, &core);
    sigemptyset(&set);
    sigaddset(&set, sig);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    raise(sig);
    exit(EMU_SIGNAL_STATUS_BASE + sig);
}

// Main function
int main(int argc, char **argv)
{
    const char *arch = EMU_DEFAULT_ARCH;
    int32_t first = argc;
    bool disasm = false;
    bool info = false;
    Emu_Trace trace = EMU_QUIET;

    Log_SetProgramName(argv[0]);

    for (int32_t i = 1; i < first; i++) {
        const char *arg = argv[i];
        if (Str_StartsWith(arg, "-m" EMU_MARCH_PREFIX)) {
            arch = arg + 2 + strlen(EMU_MARCH_PREFIX);
        } else if (Str_Equals(arg, "-d")) {
            disasm = true;
        } else if (Str_Equals(arg, "-i")) {
            info = true;
        } else if (Str_Equals(arg, "-t")) {
            trace = EMU_TRACE;
        } else if (arg[0] == '-' && arg[1]) {
            Emu_Usage(argv[0]);
        } else {
            first = i;
        }
    }

    if (first == argc) {
        Emu_Usage(argv[0]);
    }
    Err_Assert(Str_Equals(arch, EMU_DEFAULT_ARCH), ERR_EMU_ARCH_NOT_SUPPORTED, arch, EMU_DEFAULT_ARCH);

    const char *program = argv[first];
    Load_Image img = {0};
    Err_Assert(Load_ReadExec(program, &img), ERR_EMU_PROGRAM_NOT_READABLE, program, strerror(errno));
    Err_Assert(img.li_machine == ELF_EM_X86_64, ERR_EMU_ARCH_NOT_X86_64, program);

    size_t nenv = 0;
    while (environ[nenv]) {
        nenv++;
    }
    Load_PushArgs(&img, (const char *const *) argv + first, (size_t) (argc - first), (const char *const *) environ, nenv);

    int32_t status = 0;
    int32_t sig = 0;
    if (info) {
        Emu_ShowImage(&img);
    } else if (disasm) {
        Emu_Disassemble(&img);
    } else {
        status = Emu_Run(&img, trace, &sig);
    }

    Load_Free(&img);
    if (sig) {
        Emu_Raise(sig);
    }
    return status;
}
