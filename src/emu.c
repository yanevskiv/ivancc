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

// Module header.
#include "emu.h"

// The registers in the order Linux's struct sigcontext saves them.
static const Cpu_x86_64_Reg Emu_SigcontextRegs[] = {
    CPU_X86_64_REG_R8,
    CPU_X86_64_REG_R9,
    CPU_X86_64_REG_R10,
    CPU_X86_64_REG_R11,
    CPU_X86_64_REG_R12,
    CPU_X86_64_REG_R13,
    CPU_X86_64_REG_R14,
    CPU_X86_64_REG_R15,
    CPU_X86_64_REG_RDI,
    CPU_X86_64_REG_RSI,
    CPU_X86_64_REG_RBP,
    CPU_X86_64_REG_RBX,
    CPU_X86_64_REG_RDX,
    CPU_X86_64_REG_RAX,
    CPU_X86_64_REG_RCX,
    CPU_X86_64_REG_RSP
};

// Show usage information and exit.
void Emu_Usage(const char *prog)
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
void Emu_ShowImage(const Load_Image *img)
{
    fprintf(stdout, "entry  0x%llx\n", (Cpu_x86_64_TypeULLong) img->li_entry);
    fprintf(stdout, "base   0x%llx\n", (Cpu_x86_64_TypeULLong) img->li_base);
    fprintf(stdout, "size   0x%llx\n", (Cpu_x86_64_TypeULLong) img->li_size);
    fprintf(stdout, "brk    0x%llx\n", (Cpu_x86_64_TypeULLong) img->li_brk);
    fprintf(stdout, "stack  0x%llx\n", (Cpu_x86_64_TypeULLong) img->li_stack);
}

// Disassemble forward from the image's base until the bytes stop decoding.
void Emu_Disassemble(const Load_Image *img)
{
    uint64_t rip = img->li_base;
    for (;;) {
        uint64_t avail = 0;
        const uint8_t *code = Load_Span(img, rip, &avail);
        Cpu_x86_64_Insn insn;
        char text[128];

        if (! code || ! Cpu_x86_64_Decode(code, (size_t) avail, &insn)) {
            fprintf(stdout, "%016llx: (bad)\n", (Cpu_x86_64_TypeULLong) rip);
            return;
        }
        Cpu_x86_64_Format(&insn, rip, text, sizeof(text));
        fprintf(stdout, "%016llx: %s\n", (Cpu_x86_64_TypeULLong) rip, text);
        rip += insn.ci_len;
    }
}

// Return the image's memory at addr and the bytes left from there, or NULL.
uint8_t *Emu_MapMemory(void *ctx, uint64_t addr, size_t *avail)
{
    const Emu_Guest *guest = ctx;
    uint64_t span = 0;
    uint8_t *mem = Load_Span(guest->eg_img, addr, &span);
    *avail = (size_t) span;
    return mem;
}

// Read a device register, the UART being write-only and always ready.
uint64_t Emu_LoadDevice(void *ctx, uint64_t addr, size_t size)
{
    (void) ctx;
    (void) size;
    return addr == EMU_DEV_STATUS ? 1 : 0;
}

// Write a device register.
void Emu_StoreDevice(void *ctx, uint64_t addr, size_t size, uint64_t value)
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

// Store the low size bytes of value at ptr in little-endian order.
void Emu_Put(uint8_t *ptr, uint64_t value, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        ptr[i] = (uint8_t) (value >> (i * CPU_X86_64_BITS_PER_BYTE));
    }
}

// Load size little-endian bytes from ptr.
uint64_t Emu_Get(const uint8_t *ptr, size_t size)
{
    uint64_t value = 0;

    for (size_t i = 0; i < size; i++) {
        value |= (uint64_t) ptr[i] << (i * CPU_X86_64_BITS_PER_BYTE);
    }
    return value;
}

// Return the bit a signal takes in a mask.
uint64_t Emu_SignalBit(int32_t sig)
{
    return (uint64_t) 1 << (sig - EMU_SIGNAL_FIRST);
}

// True if a signal's default action is to do nothing.
bool Emu_IsDefaultIgnored(int32_t sig)
{
    return sig == SIGCHLD || sig == SIGCONT || sig == SIGURG || sig == SIGWINCH;
}

// True if a signal's default action is to stop the program.
bool Emu_IsDefaultStop(int32_t sig)
{
    return sig == SIGSTOP || sig == SIGTSTP || sig == SIGTTIN || sig == SIGTTOU;
}

// True if a signal reports a fault of the instruction that raised it.
bool Emu_IsSynchronous(int32_t sig)
{
    return sig == SIGSEGV || sig == SIGBUS || sig == SIGILL || sig == SIGTRAP || sig == SIGFPE || sig == SIGSYS;
}

// Return the signals no program can block or catch.
uint64_t Emu_Unblockable(void)
{
    return Emu_SignalBit(SIGKILL) | Emu_SignalBit(SIGSTOP);
}

// True if the program's disposition of a signal discards it.
bool Emu_IsIgnored(const Emu_Guest *guest, int32_t sig)
{
    uint64_t handler = guest->eg_action[sig].ea_handler;
    return handler == EMU_SIG_IGN || (handler == EMU_SIG_DFL && Emu_IsDefaultIgnored(sig));
}

// Send the program a signal.
void Emu_Send(Emu_Guest *guest, int32_t sig, Emu_Info info)
{
    uint64_t bit = Emu_SignalBit(sig);

    if (! (guest->eg_blocked & bit) && Emu_IsIgnored(guest, sig)) {
        return;
    }
    if (sig < EMU_SIGNAL_RT_MIN && (guest->eg_pending & bit)) {
        return;
    }
    guest->eg_pending |= bit;
    guest->eg_queued[sig]++;
    guest->eg_info[sig] = info;
}

// Send a signal the program can neither block nor ignore at that moment.
void Emu_Force(Emu_Guest *guest, int32_t sig, Emu_Info info)
{
    uint64_t bit = Emu_SignalBit(sig);

    if ((guest->eg_blocked & bit) || guest->eg_action[sig].ea_handler == EMU_SIG_IGN) {
        guest->eg_action[sig].ea_handler = EMU_SIG_DFL;
        guest->eg_blocked &= ~bit;
    }
    Emu_Send(guest, sig, info);
}

// Discard a signal's pending instances.
void Emu_Discard(Emu_Guest *guest, int32_t sig)
{
    guest->eg_pending &= ~Emu_SignalBit(sig);
    guest->eg_queued[sig] = 0;
}

// Start the program with the dispositions and mask the emulator was given.
void Emu_Inherit(Emu_Guest *guest)
{
    sigset_t set;

    sigprocmask(SIG_BLOCK, NULL, &set);
    for (int32_t sig = EMU_SIGNAL_FIRST; sig <= EMU_SIGNAL_MAX; sig++) {
        struct sigaction act;

        if (sigaction(sig, NULL, &act) == 0 && act.sa_handler == SIG_IGN) {
            guest->eg_action[sig].ea_handler = EMU_SIG_IGN;
        }
        if (sigismember(&set, sig) == 1) {
            guest->eg_blocked |= Emu_SignalBit(sig);
        }
    }
    guest->eg_blocked &= ~Emu_Unblockable();
}

// Let the program signal itself, and keep kill from reaching other processes.
uint64_t Emu_Kill(Emu_Guest *guest, int32_t pid, int32_t sig)
{
    Emu_Info info = {
        .ei_source = EMU_SOURCE_KILL,
        .ei_code   = EMU_SI_USER
    };

    if (sig < EMU_SIGNAL_NONE || sig > EMU_SIGNAL_MAX) {
        return -(uint64_t) EMU_ERRNO_INVAL;
    }
    if (pid != getpid()) {
        return -(uint64_t) EMU_ERRNO_NOSYS;
    }
    if (sig == EMU_SIGNAL_NONE) {
        return 0;
    }
    if (Emu_IsDefaultStop(sig) && guest->eg_action[sig].ea_handler == EMU_SIG_DFL) {
        return -(uint64_t) EMU_ERRNO_NOSYS;
    }
    Emu_Send(guest, sig, info);
    return 0;
}

// Set and return a signal's disposition as Linux's rt_sigaction does.
uint64_t Emu_SigAction(Emu_Guest *guest, int32_t sig, uint64_t act, uint64_t oact, uint64_t size)
{
    const uint8_t *from = Load_At(guest->eg_img, act, EMU_SIGACTION_SIZE);
    uint8_t *to = Load_At(guest->eg_img, oact, EMU_SIGACTION_SIZE);
    Emu_Action old;

    if (size != EMU_SIGSET_SIZE) {
        return -(uint64_t) EMU_ERRNO_INVAL;
    }
    if (act && ! from) {
        return -(uint64_t) EMU_ERRNO_FAULT;
    }
    if (sig < EMU_SIGNAL_FIRST || sig > EMU_SIGNAL_MAX || (act && (Emu_SignalBit(sig) & Emu_Unblockable()))) {
        return -(uint64_t) EMU_ERRNO_INVAL;
    }
    old = guest->eg_action[sig];
    if (act) {
        Emu_Action *action = &guest->eg_action[sig];
        action->ea_handler = Emu_Get(from + EMU_SIGACTION_HANDLER_OFF, sizeof(uint64_t));
        action->ea_flags = Emu_Get(from + EMU_SIGACTION_FLAGS_OFF, sizeof(uint64_t)) & EMU_SA_KNOWN;
        action->ea_restorer = Emu_Get(from + EMU_SIGACTION_RESTORER_OFF, sizeof(uint64_t));
        action->ea_mask = Emu_Get(from + EMU_SIGACTION_MASK_OFF, sizeof(uint64_t)) & ~Emu_Unblockable();
        if (Emu_IsIgnored(guest, sig)) {
            Emu_Discard(guest, sig);
        }
    }
    if (oact && ! to) {
        return -(uint64_t) EMU_ERRNO_FAULT;
    }
    if (oact) {
        Emu_Put(to + EMU_SIGACTION_HANDLER_OFF, old.ea_handler, sizeof(uint64_t));
        Emu_Put(to + EMU_SIGACTION_FLAGS_OFF, old.ea_flags, sizeof(uint64_t));
        Emu_Put(to + EMU_SIGACTION_RESTORER_OFF, old.ea_restorer, sizeof(uint64_t));
        Emu_Put(to + EMU_SIGACTION_MASK_OFF, old.ea_mask, sizeof(uint64_t));
    }
    return 0;
}

// Change and return the program's signal mask as Linux's rt_sigprocmask does.
uint64_t Emu_SigProcMask(Emu_Guest *guest, int32_t how, uint64_t set, uint64_t oset, uint64_t size)
{
    const uint8_t *from = Load_At(guest->eg_img, set, EMU_SIGSET_SIZE);
    uint8_t *to = Load_At(guest->eg_img, oset, EMU_SIGSET_SIZE);
    uint64_t old = guest->eg_blocked;

    if (size != EMU_SIGSET_SIZE) {
        return -(uint64_t) EMU_ERRNO_INVAL;
    }
    if (set && ! from) {
        return -(uint64_t) EMU_ERRNO_FAULT;
    }
    if (set) {
        uint64_t mask = Emu_Get(from, EMU_SIGSET_SIZE) & ~Emu_Unblockable();
        switch (how) {
            case EMU_SIG_BLOCK: {
                guest->eg_blocked |= mask;
            } break;
            case EMU_SIG_UNBLOCK: {
                guest->eg_blocked &= ~mask;
            } break;
            case EMU_SIG_SETMASK: {
                guest->eg_blocked = mask;
            } break;
            default: {
                return -(uint64_t) EMU_ERRNO_INVAL;
            }
        }
    }
    if (oset && ! to) {
        return -(uint64_t) EMU_ERRNO_FAULT;
    }
    if (oset) {
        Emu_Put(to, old, EMU_SIGSET_SIZE);
    }
    return 0;
}

// Return the CPU's flags as %rflags.
uint64_t Emu_ReadFlags(const Cpu_x86_64_State *cpu)
{
    uint64_t flags = EMU_RFLAGS_FIXED | EMU_RFLAGS_IF;

    flags |= cpu->cs_cf ? EMU_RFLAGS_CF : 0;
    flags |= cpu->cs_pf ? EMU_RFLAGS_PF : 0;
    flags |= cpu->cs_zf ? EMU_RFLAGS_ZF : 0;
    flags |= cpu->cs_sf ? EMU_RFLAGS_SF : 0;
    flags |= cpu->cs_of ? EMU_RFLAGS_OF : 0;
    return flags;
}

// Set the CPU's flags from %rflags.
void Emu_WriteFlags(Cpu_x86_64_State *cpu, uint64_t flags)
{
    cpu->cs_cf = (flags & EMU_RFLAGS_CF) != 0;
    cpu->cs_pf = (flags & EMU_RFLAGS_PF) != 0;
    cpu->cs_zf = (flags & EMU_RFLAGS_ZF) != 0;
    cpu->cs_sf = (flags & EMU_RFLAGS_SF) != 0;
    cpu->cs_of = (flags & EMU_RFLAGS_OF) != 0;
}

// Store the x87 and SSE registers as FXSAVE lays them out.
void Emu_SaveFpu(Cpu_x86_64_State *cpu, uint8_t *area)
{
    Emu_Put(area + EMU_FXSAVE_FCW_OFF, EMU_FCW_DEFAULT, sizeof(uint16_t));
    Emu_Put(area + EMU_FXSAVE_FSW_OFF, (uint64_t) cpu->cs_top << EMU_FSW_TOP_SHIFT, sizeof(uint16_t));
    Emu_Put(area + EMU_FXSAVE_MXCSR_OFF, EMU_MXCSR_DEFAULT, sizeof(uint32_t));
    Emu_Put(area + EMU_FXSAVE_MXCSR_MASK_OFF, EMU_MXCSR_MASK, sizeof(uint32_t));
    for (int32_t i = 0; i < CPU_X86_64_ST_COUNT; i++) {
        Fp_EncodeExtended(*Cpu_x86_64_St(cpu, i), area + EMU_FXSAVE_ST_OFF + i * EMU_FXSAVE_REG_SIZE);
    }
    for (int32_t i = 0; i < CPU_X86_64_XMM_COUNT; i++) {
        for (int32_t lane = 0; lane < CPU_X86_64_XMM_LANES; lane++) {
            Emu_Put(area + EMU_FXSAVE_XMM_OFF + i * EMU_FXSAVE_REG_SIZE + lane * sizeof(uint64_t), cpu->cs_xmm[i][lane], sizeof(uint64_t));
        }
    }
}

// Load the x87 and SSE registers from an FXSAVE area.
void Emu_RestoreFpu(Cpu_x86_64_State *cpu, const uint8_t *area)
{
    cpu->cs_top = (int32_t) (Emu_Get(area + EMU_FXSAVE_FSW_OFF, sizeof(uint16_t)) >> EMU_FSW_TOP_SHIFT) & CPU_X86_64_ST_MASK;
    for (int32_t i = 0; i < CPU_X86_64_ST_COUNT; i++) {
        *Cpu_x86_64_St(cpu, i) = Fp_DecodeExtended(area + EMU_FXSAVE_ST_OFF + i * EMU_FXSAVE_REG_SIZE);
    }
    for (int32_t i = 0; i < CPU_X86_64_XMM_COUNT; i++) {
        for (int32_t lane = 0; lane < CPU_X86_64_XMM_LANES; lane++) {
            cpu->cs_xmm[i][lane] = Emu_Get(area + EMU_FXSAVE_XMM_OFF + i * EMU_FXSAVE_REG_SIZE + lane * sizeof(uint64_t), sizeof(uint64_t));
        }
    }
}

// Put the x87 and SSE registers in the state a handler starts with.
void Emu_ResetFpu(Cpu_x86_64_State *cpu)
{
    memset(cpu->cs_xmm, 0, sizeof(cpu->cs_xmm));
    for (int32_t i = 0; i < CPU_X86_64_ST_COUNT; i++) {
        cpu->cs_st[i] = 0;
    }
    cpu->cs_top = 0;
}

// Enter a signal's handler on Linux's rt_sigframe.
bool Emu_PushFrame(Emu_Guest *guest, Cpu_x86_64_State *cpu, int32_t sig, const Emu_Action *action, const Emu_Info *info)
{
    uint64_t rsp = cpu->cs_reg[CPU_X86_64_REG_RSP];
    uint64_t fpstate = Load_AlignDown(rsp - EMU_FRAME_REDZONE - EMU_FXSAVE_SIZE, EMU_FRAME_FPSTATE_ALIGN);
    uint64_t frame = Load_AlignDown(fpstate - EMU_FRAME_SIZE, EMU_FRAME_ALIGN) - CPU_X86_64_STACK_SLOT;
    uint8_t *mem = Load_At(guest->eg_img, frame, fpstate + EMU_FXSAVE_SIZE - frame);

    if (! (action->ea_flags & EMU_SA_RESTORER) || fpstate > rsp || frame > fpstate || ! mem) {
        return false;
    }

    // Phase: the frame
    uint8_t *sc = mem + EMU_FRAME_SC_OFF;
    uint8_t *si = mem + EMU_FRAME_INFO_OFF;
    memset(mem, 0, fpstate + EMU_FXSAVE_SIZE - frame);
    Emu_Put(mem + EMU_FRAME_PRETCODE_OFF, action->ea_restorer, sizeof(uint64_t));
    Emu_Put(mem + EMU_FRAME_UC_FLAGS_OFF, EMU_UC_SIGCONTEXT_SS | EMU_UC_STRICT_RESTORE_SS, sizeof(uint64_t));
    for (size_t i = 0; i < sizeof(Emu_SigcontextRegs) / sizeof(Emu_SigcontextRegs[0]); i++) {
        Emu_Put(sc + i * sizeof(uint64_t), cpu->cs_reg[Emu_SigcontextRegs[i]], sizeof(uint64_t));
    }
    Emu_Put(sc + EMU_SC_RIP_OFF, cpu->cs_rip, sizeof(uint64_t));
    Emu_Put(sc + EMU_SC_EFLAGS_OFF, Emu_ReadFlags(cpu), sizeof(uint64_t));
    Emu_Put(sc + EMU_SC_CS_OFF, EMU_USER_CS, sizeof(uint16_t));
    Emu_Put(sc + EMU_SC_SS_OFF, EMU_USER_SS, sizeof(uint16_t));
    Emu_Put(sc + EMU_SC_ERR_OFF, guest->eg_err, sizeof(uint64_t));
    Emu_Put(sc + EMU_SC_TRAPNO_OFF, guest->eg_trapno, sizeof(uint64_t));
    Emu_Put(sc + EMU_SC_OLDMASK_OFF, guest->eg_blocked, sizeof(uint64_t));
    Emu_Put(sc + EMU_SC_CR2_OFF, cpu->cs_cr2, sizeof(uint64_t));
    Emu_Put(sc + EMU_SC_FPSTATE_OFF, fpstate, sizeof(uint64_t));
    Emu_Put(mem + EMU_FRAME_SIGMASK_OFF, guest->eg_blocked, EMU_SIGSET_SIZE);
    if (action->ea_flags & EMU_SA_SIGINFO) {
        Emu_Put(si + EMU_INFO_SIGNO_OFF, (uint64_t) sig, sizeof(uint32_t));
        Emu_Put(si + EMU_INFO_CODE_OFF, (uint64_t) info->ei_code, sizeof(uint32_t));
        if (info->ei_source == EMU_SOURCE_FAULT) {
            Emu_Put(si + EMU_INFO_ADDR_OFF, info->ei_addr, sizeof(uint64_t));
        } else if (info->ei_source == EMU_SOURCE_KILL) {
            Emu_Put(si + EMU_INFO_PID_OFF, (uint64_t) getpid(), sizeof(uint32_t));
            Emu_Put(si + EMU_INFO_UID_OFF, (uint64_t) getuid(), sizeof(uint32_t));
        }
    }
    Emu_SaveFpu(cpu, mem + (fpstate - frame));

    // Phase: the handler
    cpu->cs_reg[CPU_X86_64_REG_RDI] = (uint64_t) sig;
    cpu->cs_reg[CPU_X86_64_REG_RSI] = frame + EMU_FRAME_INFO_OFF;
    cpu->cs_reg[CPU_X86_64_REG_RDX] = frame + EMU_FRAME_UC_OFF;
    cpu->cs_reg[CPU_X86_64_REG_RAX] = 0;
    cpu->cs_reg[CPU_X86_64_REG_RSP] = frame;
    cpu->cs_rip = action->ea_handler;
    Emu_ResetFpu(cpu);
    guest->eg_blocked |= action->ea_mask;
    if (! (action->ea_flags & EMU_SA_NODEFER)) {
        guest->eg_blocked |= Emu_SignalBit(sig);
    }
    guest->eg_blocked &= ~Emu_Unblockable();
    return true;
}

// Return from a handler through its frame as Linux's rt_sigreturn does.
uint64_t Emu_SigReturn(Emu_Guest *guest, Cpu_x86_64_State *cpu)
{
    uint64_t frame = cpu->cs_reg[CPU_X86_64_REG_RSP] - CPU_X86_64_STACK_SLOT;
    const uint8_t *mem = Load_At(guest->eg_img, frame, EMU_FRAME_SIZE);
    const uint8_t *area = NULL;
    uint64_t fpstate = 0;
    Emu_Info info = {
        .ei_source = EMU_SOURCE_KERNEL,
        .ei_code   = EMU_SI_KERNEL
    };

    if (mem) {
        fpstate = Emu_Get(mem + EMU_FRAME_SC_OFF + EMU_SC_FPSTATE_OFF, sizeof(uint64_t));
        area = Load_At(guest->eg_img, fpstate, EMU_FXSAVE_SIZE);
    }
    if (! mem || (fpstate && ! area)) {
        Emu_Force(guest, SIGSEGV, info);
        return 0;
    }
    const uint8_t *sc = mem + EMU_FRAME_SC_OFF;
    guest->eg_blocked = Emu_Get(mem + EMU_FRAME_SIGMASK_OFF, EMU_SIGSET_SIZE) & ~Emu_Unblockable();
    for (size_t i = 0; i < sizeof(Emu_SigcontextRegs) / sizeof(Emu_SigcontextRegs[0]); i++) {
        cpu->cs_reg[Emu_SigcontextRegs[i]] = Emu_Get(sc + i * sizeof(uint64_t), sizeof(uint64_t));
    }
    cpu->cs_rip = Emu_Get(sc + EMU_SC_RIP_OFF, sizeof(uint64_t));
    Emu_WriteFlags(cpu, Emu_Get(sc + EMU_SC_EFLAGS_OFF, sizeof(uint64_t)));
    if (area) {
        Emu_RestoreFpu(cpu, area);
    } else {
        Emu_ResetFpu(cpu);
    }
    return cpu->cs_reg[CPU_X86_64_REG_RAX];
}

// Return the next signal to deliver.
int32_t Emu_NextSignal(const Emu_Guest *guest)
{
    uint64_t ready = guest->eg_pending & ~guest->eg_blocked;
    int32_t next = EMU_SIGNAL_NONE;

    for (int32_t sig = EMU_SIGNAL_MAX; sig >= EMU_SIGNAL_FIRST; sig--) {
        if ((ready & Emu_SignalBit(sig)) && (next == EMU_SIGNAL_NONE || ! Emu_IsSynchronous(next) || Emu_IsSynchronous(sig))) {
            next = sig;
        }
    }
    return next;
}

// Deliver every pending signal the program does not block.
void Emu_Deliver(Emu_Guest *guest, Cpu_x86_64_State *cpu)
{
    Emu_Info kernel = {
        .ei_source = EMU_SOURCE_KERNEL,
        .ei_code   = EMU_SI_KERNEL
    };

    while (! guest->eg_halted) {
        int32_t sig = Emu_NextSignal(guest);
        if (sig == EMU_SIGNAL_NONE) {
            return;
        }

        // Phase: take one instance
        Emu_Info info = guest->eg_info[sig];
        Emu_Action action = guest->eg_action[sig];
        guest->eg_queued[sig]--;
        if (guest->eg_queued[sig] == 0) {
            guest->eg_pending &= ~Emu_SignalBit(sig);
        }

        // Phase: act on it
        if (action.ea_handler == EMU_SIG_IGN || (action.ea_handler == EMU_SIG_DFL && (Emu_IsDefaultIgnored(sig) || Emu_IsDefaultStop(sig)))) {
            continue;
        }
        if (action.ea_handler == EMU_SIG_DFL) {
            if (info.ei_source == EMU_SOURCE_FAULT && cpu->cs_fault) {
                Log_Show(LOG_SEVERITY_ERROR, LOG_LINE_NONE, "%s", cpu->cs_fault);
            }
            guest->eg_halted = true;
            guest->eg_signal = sig;
            return;
        }
        if (action.ea_flags & EMU_SA_RESETHAND) {
            guest->eg_action[sig].ea_handler = EMU_SIG_DFL;
        }
        if (! Emu_PushFrame(guest, cpu, sig, &action, &info)) {
            if (sig == SIGSEGV) {
                guest->eg_action[sig].ea_handler = EMU_SIG_DFL;
            }
            Emu_Force(guest, SIGSEGV, kernel);
        }
    }
}

// Turn the CPU's exception into the signal Linux sends for it.
void Emu_Fault(Emu_Guest *guest, Cpu_x86_64_State *cpu, uint64_t rip)
{
    Emu_Info info = {
        .ei_source = EMU_SOURCE_FAULT,
        .ei_addr   = rip
    };
    int32_t sig = SIGSEGV;

    cpu->cs_rip = rip;
    guest->eg_trapno = cpu->cs_vector;
    guest->eg_err = 0;
    switch (cpu->cs_vector) {
        case CPU_X86_64_VECTOR_DE: {
            sig = SIGFPE;
            info.ei_code = EMU_FPE_INTDIV;
        } break;
        case CPU_X86_64_VECTOR_UD: {
            sig = SIGILL;
            info.ei_code = EMU_ILL_ILLOPN;
        } break;
        default: {
            guest->eg_err = cpu->cs_pf_err;
            info.ei_code = EMU_SEGV_MAPERR;
            info.ei_addr = cpu->cs_cr2;
        }
    }
    Emu_Force(guest, sig, info);
}

// Move the break as Linux's brk does, zeroing the pages a shrink gives back.
uint64_t Emu_Brk(Emu_Guest *guest, uint64_t addr)
{
    Load_Image *img = guest->eg_img;
    if (addr < img->li_brk_base || addr > img->li_heap_end) {
        return img->li_brk;
    }

    uint64_t keep = Load_AlignUp(addr, ELF_PAGE);
    uint64_t mapped = Load_AlignUp(img->li_brk, ELF_PAGE);
    if (keep < mapped) {
        memset(img->li_mem + (keep - img->li_base), 0, mapped - keep);
    }
    img->li_brk = addr;
    return addr;
}

// Read the host's clock into the program's struct timespec at addr.
uint64_t Emu_ClockGettime(Emu_Guest *guest, int32_t clock, uint64_t addr)
{
    uint8_t *spec = Load_At(guest->eg_img, addr, EMU_TIMESPEC_SIZE);
    struct timespec now;

    if (clock_gettime((clockid_t) clock, &now) != 0) {
        return -(uint64_t) errno;
    }
    if (! spec) {
        return -(uint64_t) EMU_ERRNO_FAULT;
    }
    Load_PutWord(spec + EMU_TIMESPEC_SEC_OFF, (uint64_t) now.tv_sec);
    Load_PutWord(spec + EMU_TIMESPEC_NSEC_OFF, (uint64_t) now.tv_nsec);
    return 0;
}

// Answer a syscall as Linux does and fail the ones it lacks with ENOSYS.
void Emu_Syscall(Emu_Guest *guest, Cpu_x86_64_State *cpu)
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
        case EMU_SYS_BRK: {
            *rax = Emu_Brk(guest, cpu->cs_reg[CPU_X86_64_REG_RDI]);
        } break;
        case EMU_SYS_RT_SIGACTION: {
            int32_t sig = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            uint64_t act = cpu->cs_reg[CPU_X86_64_REG_RSI];
            uint64_t oact = cpu->cs_reg[CPU_X86_64_REG_RDX];
            uint64_t size = cpu->cs_reg[CPU_X86_64_REG_R10];
            *rax = Emu_SigAction(guest, sig, act, oact, size);
        } break;
        case EMU_SYS_RT_SIGPROCMASK: {
            int32_t how = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            uint64_t set = cpu->cs_reg[CPU_X86_64_REG_RSI];
            uint64_t oset = cpu->cs_reg[CPU_X86_64_REG_RDX];
            uint64_t size = cpu->cs_reg[CPU_X86_64_REG_R10];
            *rax = Emu_SigProcMask(guest, how, set, oset, size);
        } break;
        case EMU_SYS_RT_SIGRETURN: {
            *rax = Emu_SigReturn(guest, cpu);
        } break;
        case EMU_SYS_GETPID: {
            *rax = (uint64_t) getpid();
        } break;
        case EMU_SYS_EXIT:
        case EMU_SYS_EXIT_GROUP: {
            guest->eg_halted = true;
            guest->eg_status = cpu->cs_reg[CPU_X86_64_REG_RDI] & CPU_X86_64_MASK_8;
        } break;
        case EMU_SYS_KILL: {
            int32_t pid = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            int32_t sig = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RSI];
            *rax = Emu_Kill(guest, pid, sig);
        } break;
        case EMU_SYS_CLOCK_GETTIME: {
            int32_t clock = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            uint64_t addr = cpu->cs_reg[CPU_X86_64_REG_RSI];
            *rax = Emu_ClockGettime(guest, clock, addr);
        } break;
        default: {
            *rax = -(uint64_t) EMU_ERRNO_NOSYS;
        }
    }
}

// Write the instruction at rip to stderr before it runs, if it decodes.
void Emu_ShowStep(Emu_Guest *guest, uint64_t rip)
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

// Run a loaded program to completion; return its status and ending signal.
int32_t Emu_Run(Load_Image *img, Emu_Trace trace, int32_t *sig)
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
    Emu_Inherit(&guest);
    while (! guest.eg_halted) {
        uint64_t rip = cpu.cs_rip;

        if (trace == EMU_TRACE) {
            Emu_ShowStep(&guest, rip);
        }
        Cpu_x86_64_Step(&cpu);
        switch (cpu.cs_trap) {
            case CPU_X86_64_TRAP_SYSCALL: {
                Emu_Syscall(&guest, &cpu);
                Emu_Deliver(&guest, &cpu);
            } break;
            case CPU_X86_64_TRAP_EXCEPTION: {
                Emu_Fault(&guest, &cpu, rip);
                Emu_Deliver(&guest, &cpu);
            } break;
            default: {
                // empty
            } break;
        }
    }
    Cpu_x86_64_Free(&cpu);
    *sig = guest.eg_signal;
    return guest.eg_status;
}

// Die by the signal that ended a program as the program would have died.
void Emu_Raise(int32_t sig)
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
    uint64_t end = img.li_base + img.li_size;
    Err_Assert(end <= EMU_DEV_BASE || img.li_base >= EMU_DEV_BASE + EMU_DEV_SIZE, ERR_EMU_IMAGE_OVER_DEVICES, program, (Cpu_x86_64_TypeULLong) img.li_base, (Cpu_x86_64_TypeULLong) end, (Cpu_x86_64_TypeULLong) EMU_DEV_BASE);

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
