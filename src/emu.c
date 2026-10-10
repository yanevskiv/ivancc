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
static const Cpu_x86_64_Reg Emu_x86_64_Linux_SigcontextRegs[] = {
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

// Return the image's memory at addr and the bytes left from there, or NULL.
uint8_t *Emu_MapMemory(void *ctx, uint64_t addr, size_t *avail)
{
    const Emu_x86_64_Linux_Guest *guest = ctx;
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
    Emu_x86_64_Linux_Guest *guest = ctx;

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

// Return the bit a signal takes in a mask.
uint64_t Emu_Linux_SignalBit(int32_t sig)
{
    return (uint64_t) 1 << (sig - EMU_LINUX_SIGNAL_FIRST);
}

// True if a signal's default action is to do nothing.
bool Emu_Linux_IsDefaultIgnored(int32_t sig)
{
    return sig == SIGCHLD || sig == SIGCONT || sig == SIGURG || sig == SIGWINCH;
}

// True if a signal's default action is to stop the program.
bool Emu_Linux_IsDefaultStop(int32_t sig)
{
    return sig == SIGSTOP || sig == SIGTSTP || sig == SIGTTIN || sig == SIGTTOU;
}

// True if a signal reports a fault of the instruction that raised it.
bool Emu_Linux_IsSynchronous(int32_t sig)
{
    return sig == SIGSEGV || sig == SIGBUS || sig == SIGILL || sig == SIGTRAP || sig == SIGFPE || sig == SIGSYS;
}

// Return the signals no program can block or catch.
uint64_t Emu_Linux_Unblockable(void)
{
    return Emu_Linux_SignalBit(SIGKILL) | Emu_Linux_SignalBit(SIGSTOP);
}

// True if the program's disposition of a signal discards it.
bool Emu_Linux_IsIgnored(const Emu_x86_64_Linux_Guest *guest, int32_t sig)
{
    uint64_t handler = guest->eg_action[sig].ea_handler;
    return handler == EMU_LINUX_SIG_IGN || (handler == EMU_LINUX_SIG_DFL && Emu_Linux_IsDefaultIgnored(sig));
}

// Send the program a signal.
void Emu_Linux_Send(Emu_x86_64_Linux_Guest *guest, int32_t sig, Emu_Linux_Info info)
{
    uint64_t bit = Emu_Linux_SignalBit(sig);

    if (! (guest->eg_blocked & bit) && Emu_Linux_IsIgnored(guest, sig)) {
        return;
    }
    if (sig < EMU_LINUX_SIGNAL_RT_MIN && (guest->eg_pending & bit)) {
        return;
    }
    guest->eg_pending |= bit;
    guest->eg_queued[sig]++;
    guest->eg_info[sig] = info;
}

// Send a signal the program can neither block nor ignore at that moment.
void Emu_Linux_Force(Emu_x86_64_Linux_Guest *guest, int32_t sig, Emu_Linux_Info info)
{
    uint64_t bit = Emu_Linux_SignalBit(sig);

    if ((guest->eg_blocked & bit) || guest->eg_action[sig].ea_handler == EMU_LINUX_SIG_IGN) {
        guest->eg_action[sig].ea_handler = EMU_LINUX_SIG_DFL;
        guest->eg_blocked &= ~bit;
    }
    Emu_Linux_Send(guest, sig, info);
}

// Discard a signal's pending instances.
void Emu_Linux_Discard(Emu_x86_64_Linux_Guest *guest, int32_t sig)
{
    guest->eg_pending &= ~Emu_Linux_SignalBit(sig);
    guest->eg_queued[sig] = 0;
}

// Start the program with the dispositions and mask the emulator was given.
void Emu_Linux_Inherit(Emu_x86_64_Linux_Guest *guest)
{
    sigset_t set;

    sigprocmask(SIG_BLOCK, NULL, &set);
    for (int32_t sig = EMU_LINUX_SIGNAL_FIRST; sig <= EMU_LINUX_SIGNAL_MAX; sig++) {
        struct sigaction act;

        if (sigaction(sig, NULL, &act) == 0 && act.sa_handler == SIG_IGN) {
            guest->eg_action[sig].ea_handler = EMU_LINUX_SIG_IGN;
        }
        if (sigismember(&set, sig) == 1) {
            guest->eg_blocked |= Emu_Linux_SignalBit(sig);
        }
    }
    guest->eg_blocked &= ~Emu_Linux_Unblockable();
}

// Let the program signal itself, and keep kill from reaching other processes.
uint64_t Emu_Linux_Kill(Emu_x86_64_Linux_Guest *guest, int32_t pid, int32_t sig)
{
    Emu_Linux_Info info = {
        .ei_source = EMU_SOURCE_KILL,
        .ei_code   = EMU_LINUX_SI_USER
    };

    if (sig < EMU_LINUX_SIGNAL_NONE || sig > EMU_LINUX_SIGNAL_MAX) {
        return -(uint64_t) EMU_LINUX_ERRNO_INVAL;
    }
    if (pid != getpid()) {
        return -(uint64_t) EMU_LINUX_ERRNO_NOSYS;
    }
    if (sig == EMU_LINUX_SIGNAL_NONE) {
        return 0;
    }
    if (Emu_Linux_IsDefaultStop(sig) && guest->eg_action[sig].ea_handler == EMU_LINUX_SIG_DFL) {
        return -(uint64_t) EMU_LINUX_ERRNO_NOSYS;
    }
    Emu_Linux_Send(guest, sig, info);
    return 0;
}

// Change and return the program's signal mask as Linux's rt_sigprocmask does.
uint64_t Emu_Linux_SigProcMask(Emu_x86_64_Linux_Guest *guest, int32_t how, uint64_t set, uint64_t oset, uint64_t size)
{
    const uint8_t *from = Load_At(guest->eg_img, set, EMU_LINUX_SIGSET_SIZE);
    uint8_t *to = Load_At(guest->eg_img, oset, EMU_LINUX_SIGSET_SIZE);
    uint64_t old = guest->eg_blocked;

    if (size != EMU_LINUX_SIGSET_SIZE) {
        return -(uint64_t) EMU_LINUX_ERRNO_INVAL;
    }
    if (set && ! from) {
        return -(uint64_t) EMU_LINUX_ERRNO_FAULT;
    }
    if (set) {
        uint64_t mask = Emu_Get(from, EMU_LINUX_SIGSET_SIZE) & ~Emu_Linux_Unblockable();
        switch (how) {
            case EMU_LINUX_SIG_BLOCK: {
                guest->eg_blocked |= mask;
            } break;
            case EMU_LINUX_SIG_UNBLOCK: {
                guest->eg_blocked &= ~mask;
            } break;
            case EMU_LINUX_SIG_SETMASK: {
                guest->eg_blocked = mask;
            } break;
            default: {
                return -(uint64_t) EMU_LINUX_ERRNO_INVAL;
            }
        }
    }
    if (oset && ! to) {
        return -(uint64_t) EMU_LINUX_ERRNO_FAULT;
    }
    if (oset) {
        Emu_Put(to, old, EMU_LINUX_SIGSET_SIZE);
    }
    return 0;
}

// Return the next signal to deliver.
int32_t Emu_Linux_NextSignal(const Emu_x86_64_Linux_Guest *guest)
{
    uint64_t ready = guest->eg_pending & ~guest->eg_blocked;
    int32_t next = EMU_LINUX_SIGNAL_NONE;

    for (int32_t sig = EMU_LINUX_SIGNAL_MAX; sig >= EMU_LINUX_SIGNAL_FIRST; sig--) {
        if ((ready & Emu_Linux_SignalBit(sig)) && (next == EMU_LINUX_SIGNAL_NONE || ! Emu_Linux_IsSynchronous(next) || Emu_Linux_IsSynchronous(sig))) {
            next = sig;
        }
    }
    return next;
}

// Return a host call's result as a syscall returns it, its errno negated.
uint64_t Emu_Linux_Result(int64_t ret)
{
    return ret < 0 ? -(uint64_t) errno : (uint64_t) ret;
}

// Point path at the program's string at addr as Linux's getname takes it.
uint64_t Emu_Linux_Path(const Emu_x86_64_Linux_Guest *guest, uint64_t addr, const char **path)
{
    uint64_t avail = 0;
    const uint8_t *mem = Load_Span(guest->eg_img, addr, &avail);
    size_t span = avail < EMU_LINUX_PATH_MAX ? (size_t) avail : EMU_LINUX_PATH_MAX;

    if (! mem) {
        return -(uint64_t) EMU_LINUX_ERRNO_FAULT;
    }
    if (! memchr(mem, 0, span)) {
        return -(uint64_t) (span < EMU_LINUX_PATH_MAX ? EMU_LINUX_ERRNO_FAULT : EMU_LINUX_ERRNO_NAMETOOLONG);
    }
    *path = (const char *) mem;
    return 0;
}

// Read from the host's descriptor into the program's buffer at buf.
uint64_t Emu_Linux_Read(Emu_x86_64_Linux_Guest *guest, int32_t fd, uint64_t buf, uint64_t len)
{
    uint8_t *mem = Load_At(guest->eg_img, buf, len);

    if (! mem && len != 0) {
        return -(uint64_t) EMU_LINUX_ERRNO_FAULT;
    }
    return Emu_Linux_Result(read(fd, mem, (size_t) len));
}

// Write the program's buffer at buf to the host's descriptor.
uint64_t Emu_Linux_Write(Emu_x86_64_Linux_Guest *guest, int32_t fd, uint64_t buf, uint64_t len)
{
    const uint8_t *mem = Load_At(guest->eg_img, buf, len);

    if (! mem && len != 0) {
        return -(uint64_t) EMU_LINUX_ERRNO_FAULT;
    }
    return Emu_Linux_Result(write(fd, mem, (size_t) len));
}

// Open the program's path on the host, its flags and mode as they are.
uint64_t Emu_Linux_Open(Emu_x86_64_Linux_Guest *guest, uint64_t addr, int32_t flags, uint32_t mode)
{
    const char *path = NULL;
    uint64_t err = Emu_Linux_Path(guest, addr, &path);

    if (err != 0) {
        return err;
    }
    return Emu_Linux_Result(open(path, flags, (mode_t) mode));
}

// Move the break as Linux's brk does, zeroing the pages a shrink gives back.
uint64_t Emu_Linux_Brk(Emu_x86_64_Linux_Guest *guest, uint64_t addr)
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

// Answer TCGETS from the host's terminal, and fail every other request.
uint64_t Emu_Linux_Ioctl(Emu_x86_64_Linux_Guest *guest, int32_t fd, uint32_t req, uint64_t arg)
{
    uint8_t *mem = Load_At(guest->eg_img, arg, EMU_LINUX_TERMIOS_SIZE);
    uint8_t termios[EMU_LINUX_TERMIOS_SIZE];

    if (req != EMU_LINUX_IOCTL_TCGETS) {
        return fcntl(fd, F_GETFD) < 0 ? -(uint64_t) errno : -(uint64_t) EMU_LINUX_ERRNO_NOTTY;
    }
    if (ioctl(fd, EMU_LINUX_IOCTL_TCGETS, termios) < 0) {
        return -(uint64_t) errno;
    }
    if (! mem) {
        return -(uint64_t) EMU_LINUX_ERRNO_FAULT;
    }
    memcpy(mem, termios, sizeof(termios));
    return 0;
}

// Rename the program's path at from to its path at to, on the host.
uint64_t Emu_Linux_Rename(Emu_x86_64_Linux_Guest *guest, uint64_t from, uint64_t to)
{
    const char *src = NULL;
    const char *dst = NULL;
    uint64_t err = Emu_Linux_Path(guest, from, &src);

    if (err != 0) {
        return err;
    }
    err = Emu_Linux_Path(guest, to, &dst);
    if (err != 0) {
        return err;
    }
    return Emu_Linux_Result(rename(src, dst));
}

// Make the program's directory at addr on the host, its mode as it is.
uint64_t Emu_Linux_Mkdir(Emu_x86_64_Linux_Guest *guest, uint64_t addr, uint32_t mode)
{
    const char *path = NULL;
    uint64_t err = Emu_Linux_Path(guest, addr, &path);

    if (err != 0) {
        return err;
    }
    return Emu_Linux_Result(mkdir(path, (mode_t) mode));
}

// Remove the program's empty directory at addr from the host.
uint64_t Emu_Linux_Rmdir(Emu_x86_64_Linux_Guest *guest, uint64_t addr)
{
    const char *path = NULL;
    uint64_t err = Emu_Linux_Path(guest, addr, &path);

    if (err != 0) {
        return err;
    }
    return Emu_Linux_Result(rmdir(path));
}

// Remove the program's file at addr from the host.
uint64_t Emu_Linux_Unlink(Emu_x86_64_Linux_Guest *guest, uint64_t addr)
{
    const char *path = NULL;
    uint64_t err = Emu_Linux_Path(guest, addr, &path);

    if (err != 0) {
        return err;
    }
    return Emu_Linux_Result(unlink(path));
}

// Read the host's clock into the program's struct timespec at addr.
uint64_t Emu_Linux_ClockGettime(Emu_x86_64_Linux_Guest *guest, int32_t clock, uint64_t addr)
{
    uint8_t *spec = Load_At(guest->eg_img, addr, EMU_LINUX_TIMESPEC_SIZE);
    struct timespec now;

    if (clock_gettime((clockid_t) clock, &now) != 0) {
        return -(uint64_t) errno;
    }
    if (! spec) {
        return -(uint64_t) EMU_LINUX_ERRNO_FAULT;
    }
    Load_PutWord(spec + EMU_LINUX_TIMESPEC_SEC_OFF, (uint64_t) now.tv_sec);
    Load_PutWord(spec + EMU_LINUX_TIMESPEC_NSEC_OFF, (uint64_t) now.tv_nsec);
    return 0;
}

// Disassemble forward from the image's base until the bytes stop decoding.
void Emu_x86_64_Disassemble(const Load_Image *img)
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

// Write the instruction at rip to stderr before it runs, if it decodes.
void Emu_x86_64_ShowStep(Emu_x86_64_Linux_Guest *guest, uint64_t rip)
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

// Return the CPU's flags as %rflags.
uint64_t Emu_x86_64_ReadFlags(const Cpu_x86_64_State *cpu)
{
    uint64_t flags = EMU_X86_64_RFLAGS_FIXED | EMU_X86_64_RFLAGS_IF;

    flags |= cpu->cs_cf ? EMU_X86_64_RFLAGS_CF : 0;
    flags |= cpu->cs_pf ? EMU_X86_64_RFLAGS_PF : 0;
    flags |= cpu->cs_zf ? EMU_X86_64_RFLAGS_ZF : 0;
    flags |= cpu->cs_sf ? EMU_X86_64_RFLAGS_SF : 0;
    flags |= cpu->cs_of ? EMU_X86_64_RFLAGS_OF : 0;
    return flags;
}

// Set the CPU's flags from %rflags.
void Emu_x86_64_WriteFlags(Cpu_x86_64_State *cpu, uint64_t flags)
{
    cpu->cs_cf = (flags & EMU_X86_64_RFLAGS_CF) != 0;
    cpu->cs_pf = (flags & EMU_X86_64_RFLAGS_PF) != 0;
    cpu->cs_zf = (flags & EMU_X86_64_RFLAGS_ZF) != 0;
    cpu->cs_sf = (flags & EMU_X86_64_RFLAGS_SF) != 0;
    cpu->cs_of = (flags & EMU_X86_64_RFLAGS_OF) != 0;
}

// Store the x87 and SSE registers as FXSAVE lays them out.
void Emu_x86_64_SaveFpu(Cpu_x86_64_State *cpu, uint8_t *area)
{
    Emu_Put(area + EMU_X86_64_FXSAVE_FCW_OFF, EMU_X86_64_FCW_DEFAULT, sizeof(uint16_t));
    Emu_Put(area + EMU_X86_64_FXSAVE_FSW_OFF, (uint64_t) cpu->cs_top << EMU_X86_64_FSW_TOP_SHIFT, sizeof(uint16_t));
    Emu_Put(area + EMU_X86_64_FXSAVE_MXCSR_OFF, EMU_X86_64_MXCSR_DEFAULT, sizeof(uint32_t));
    Emu_Put(area + EMU_X86_64_FXSAVE_MXCSR_MASK_OFF, EMU_X86_64_MXCSR_MASK, sizeof(uint32_t));
    for (int32_t i = 0; i < CPU_X86_64_ST_COUNT; i++) {
        Fp_EncodeExtended(*Cpu_x86_64_St(cpu, i), area + EMU_X86_64_FXSAVE_ST_OFF + i * EMU_X86_64_FXSAVE_REG_SIZE);
    }
    for (int32_t i = 0; i < CPU_X86_64_XMM_COUNT; i++) {
        for (int32_t lane = 0; lane < CPU_X86_64_XMM_LANES; lane++) {
            Emu_Put(area + EMU_X86_64_FXSAVE_XMM_OFF + i * EMU_X86_64_FXSAVE_REG_SIZE + lane * sizeof(uint64_t), cpu->cs_xmm[i][lane], sizeof(uint64_t));
        }
    }
}

// Load the x87 and SSE registers from an FXSAVE area.
void Emu_x86_64_RestoreFpu(Cpu_x86_64_State *cpu, const uint8_t *area)
{
    cpu->cs_top = (int32_t) (Emu_Get(area + EMU_X86_64_FXSAVE_FSW_OFF, sizeof(uint16_t)) >> EMU_X86_64_FSW_TOP_SHIFT) & CPU_X86_64_ST_MASK;
    for (int32_t i = 0; i < CPU_X86_64_ST_COUNT; i++) {
        *Cpu_x86_64_St(cpu, i) = Fp_DecodeExtended(area + EMU_X86_64_FXSAVE_ST_OFF + i * EMU_X86_64_FXSAVE_REG_SIZE);
    }
    for (int32_t i = 0; i < CPU_X86_64_XMM_COUNT; i++) {
        for (int32_t lane = 0; lane < CPU_X86_64_XMM_LANES; lane++) {
            cpu->cs_xmm[i][lane] = Emu_Get(area + EMU_X86_64_FXSAVE_XMM_OFF + i * EMU_X86_64_FXSAVE_REG_SIZE + lane * sizeof(uint64_t), sizeof(uint64_t));
        }
    }
}

// Put the x87 and SSE registers in the state a handler starts with.
void Emu_x86_64_ResetFpu(Cpu_x86_64_State *cpu)
{
    memset(cpu->cs_xmm, 0, sizeof(cpu->cs_xmm));
    for (int32_t i = 0; i < CPU_X86_64_ST_COUNT; i++) {
        cpu->cs_st[i] = 0;
    }
    cpu->cs_top = 0;
}

// Set and return a signal's disposition as Linux's rt_sigaction does.
uint64_t Emu_x86_64_Linux_SigAction(Emu_x86_64_Linux_Guest *guest, int32_t sig, uint64_t act, uint64_t oact, uint64_t size)
{
    const uint8_t *from = Load_At(guest->eg_img, act, EMU_X86_64_LINUX_SIGACTION_SIZE);
    uint8_t *to = Load_At(guest->eg_img, oact, EMU_X86_64_LINUX_SIGACTION_SIZE);
    Emu_Linux_Action old;

    if (size != EMU_LINUX_SIGSET_SIZE) {
        return -(uint64_t) EMU_LINUX_ERRNO_INVAL;
    }
    if (act && ! from) {
        return -(uint64_t) EMU_LINUX_ERRNO_FAULT;
    }
    if (sig < EMU_LINUX_SIGNAL_FIRST || sig > EMU_LINUX_SIGNAL_MAX || (act && (Emu_Linux_SignalBit(sig) & Emu_Linux_Unblockable()))) {
        return -(uint64_t) EMU_LINUX_ERRNO_INVAL;
    }
    old = guest->eg_action[sig];
    if (act) {
        Emu_Linux_Action *action = &guest->eg_action[sig];
        action->ea_handler = Emu_Get(from + EMU_X86_64_LINUX_SIGACTION_HANDLER_OFF, sizeof(uint64_t));
        action->ea_flags = Emu_Get(from + EMU_X86_64_LINUX_SIGACTION_FLAGS_OFF, sizeof(uint64_t)) & EMU_X86_64_LINUX_SA_KNOWN;
        action->ea_restorer = Emu_Get(from + EMU_X86_64_LINUX_SIGACTION_RESTORER_OFF, sizeof(uint64_t));
        action->ea_mask = Emu_Get(from + EMU_X86_64_LINUX_SIGACTION_MASK_OFF, sizeof(uint64_t)) & ~Emu_Linux_Unblockable();
        if (Emu_Linux_IsIgnored(guest, sig)) {
            Emu_Linux_Discard(guest, sig);
        }
    }
    if (oact && ! to) {
        return -(uint64_t) EMU_LINUX_ERRNO_FAULT;
    }
    if (oact) {
        Emu_Put(to + EMU_X86_64_LINUX_SIGACTION_HANDLER_OFF, old.ea_handler, sizeof(uint64_t));
        Emu_Put(to + EMU_X86_64_LINUX_SIGACTION_FLAGS_OFF, old.ea_flags, sizeof(uint64_t));
        Emu_Put(to + EMU_X86_64_LINUX_SIGACTION_RESTORER_OFF, old.ea_restorer, sizeof(uint64_t));
        Emu_Put(to + EMU_X86_64_LINUX_SIGACTION_MASK_OFF, old.ea_mask, sizeof(uint64_t));
    }
    return 0;
}

// Enter a signal's handler on Linux's rt_sigframe.
bool Emu_x86_64_Linux_PushFrame(Emu_x86_64_Linux_Guest *guest, Cpu_x86_64_State *cpu, int32_t sig, const Emu_Linux_Action *action, const Emu_Linux_Info *info)
{
    uint64_t rsp = cpu->cs_reg[CPU_X86_64_REG_RSP];
    uint64_t fpstate = Load_AlignDown(rsp - EMU_X86_64_LINUX_FRAME_REDZONE - EMU_X86_64_FXSAVE_SIZE, EMU_X86_64_LINUX_FRAME_FPSTATE_ALIGN);
    uint64_t frame = Load_AlignDown(fpstate - EMU_X86_64_LINUX_FRAME_SIZE, EMU_X86_64_LINUX_FRAME_ALIGN) - CPU_X86_64_STACK_SLOT;
    uint8_t *mem = Load_At(guest->eg_img, frame, fpstate + EMU_X86_64_FXSAVE_SIZE - frame);

    if (! (action->ea_flags & EMU_X86_64_LINUX_SA_RESTORER) || fpstate > rsp || frame > fpstate || ! mem) {
        return false;
    }

    // Phase: the frame
    uint8_t *sc = mem + EMU_X86_64_LINUX_FRAME_SC_OFF;
    uint8_t *si = mem + EMU_X86_64_LINUX_FRAME_INFO_OFF;
    memset(mem, 0, fpstate + EMU_X86_64_FXSAVE_SIZE - frame);
    Emu_Put(mem + EMU_X86_64_LINUX_FRAME_PRETCODE_OFF, action->ea_restorer, sizeof(uint64_t));
    Emu_Put(mem + EMU_X86_64_LINUX_FRAME_UC_FLAGS_OFF, EMU_X86_64_LINUX_UC_SIGCONTEXT_SS | EMU_X86_64_LINUX_UC_STRICT_RESTORE_SS, sizeof(uint64_t));
    for (size_t i = 0; i < sizeof(Emu_x86_64_Linux_SigcontextRegs) / sizeof(Emu_x86_64_Linux_SigcontextRegs[0]); i++) {
        Emu_Put(sc + i * sizeof(uint64_t), cpu->cs_reg[Emu_x86_64_Linux_SigcontextRegs[i]], sizeof(uint64_t));
    }
    Emu_Put(sc + EMU_X86_64_LINUX_SC_RIP_OFF, cpu->cs_rip, sizeof(uint64_t));
    Emu_Put(sc + EMU_X86_64_LINUX_SC_EFLAGS_OFF, Emu_x86_64_ReadFlags(cpu), sizeof(uint64_t));
    Emu_Put(sc + EMU_X86_64_LINUX_SC_CS_OFF, EMU_X86_64_LINUX_USER_CS, sizeof(uint16_t));
    Emu_Put(sc + EMU_X86_64_LINUX_SC_SS_OFF, EMU_X86_64_LINUX_USER_SS, sizeof(uint16_t));
    Emu_Put(sc + EMU_X86_64_LINUX_SC_ERR_OFF, guest->eg_err, sizeof(uint64_t));
    Emu_Put(sc + EMU_X86_64_LINUX_SC_TRAPNO_OFF, guest->eg_trapno, sizeof(uint64_t));
    Emu_Put(sc + EMU_X86_64_LINUX_SC_OLDMASK_OFF, guest->eg_blocked, sizeof(uint64_t));
    Emu_Put(sc + EMU_X86_64_LINUX_SC_CR2_OFF, cpu->cs_cr2, sizeof(uint64_t));
    Emu_Put(sc + EMU_X86_64_LINUX_SC_FPSTATE_OFF, fpstate, sizeof(uint64_t));
    Emu_Put(mem + EMU_X86_64_LINUX_FRAME_SIGMASK_OFF, guest->eg_blocked, EMU_LINUX_SIGSET_SIZE);
    if (action->ea_flags & EMU_X86_64_LINUX_SA_SIGINFO) {
        Emu_Put(si + EMU_LINUX_INFO_SIGNO_OFF, (uint64_t) sig, sizeof(uint32_t));
        Emu_Put(si + EMU_LINUX_INFO_CODE_OFF, (uint64_t) info->ei_code, sizeof(uint32_t));
        if (info->ei_source == EMU_SOURCE_FAULT) {
            Emu_Put(si + EMU_LINUX_INFO_ADDR_OFF, info->ei_addr, sizeof(uint64_t));
        } else if (info->ei_source == EMU_SOURCE_KILL) {
            Emu_Put(si + EMU_LINUX_INFO_PID_OFF, (uint64_t) getpid(), sizeof(uint32_t));
            Emu_Put(si + EMU_LINUX_INFO_UID_OFF, (uint64_t) getuid(), sizeof(uint32_t));
        }
    }
    Emu_x86_64_SaveFpu(cpu, mem + (fpstate - frame));

    // Phase: the handler
    cpu->cs_reg[CPU_X86_64_REG_RDI] = (uint64_t) sig;
    cpu->cs_reg[CPU_X86_64_REG_RSI] = frame + EMU_X86_64_LINUX_FRAME_INFO_OFF;
    cpu->cs_reg[CPU_X86_64_REG_RDX] = frame + EMU_X86_64_LINUX_FRAME_UC_OFF;
    cpu->cs_reg[CPU_X86_64_REG_RAX] = 0;
    cpu->cs_reg[CPU_X86_64_REG_RSP] = frame;
    cpu->cs_rip = action->ea_handler;
    Emu_x86_64_ResetFpu(cpu);
    guest->eg_blocked |= action->ea_mask;
    if (! (action->ea_flags & EMU_X86_64_LINUX_SA_NODEFER)) {
        guest->eg_blocked |= Emu_Linux_SignalBit(sig);
    }
    guest->eg_blocked &= ~Emu_Linux_Unblockable();
    return true;
}

// Return from a handler through its frame as Linux's rt_sigreturn does.
uint64_t Emu_x86_64_Linux_SigReturn(Emu_x86_64_Linux_Guest *guest, Cpu_x86_64_State *cpu)
{
    uint64_t frame = cpu->cs_reg[CPU_X86_64_REG_RSP] - CPU_X86_64_STACK_SLOT;
    const uint8_t *mem = Load_At(guest->eg_img, frame, EMU_X86_64_LINUX_FRAME_SIZE);
    const uint8_t *area = NULL;
    uint64_t fpstate = 0;
    Emu_Linux_Info info = {
        .ei_source = EMU_SOURCE_KERNEL,
        .ei_code   = EMU_LINUX_SI_KERNEL
    };

    if (mem) {
        fpstate = Emu_Get(mem + EMU_X86_64_LINUX_FRAME_SC_OFF + EMU_X86_64_LINUX_SC_FPSTATE_OFF, sizeof(uint64_t));
        area = Load_At(guest->eg_img, fpstate, EMU_X86_64_FXSAVE_SIZE);
    }
    if (! mem || (fpstate && ! area)) {
        Emu_Linux_Force(guest, SIGSEGV, info);
        return 0;
    }
    const uint8_t *sc = mem + EMU_X86_64_LINUX_FRAME_SC_OFF;
    guest->eg_blocked = Emu_Get(mem + EMU_X86_64_LINUX_FRAME_SIGMASK_OFF, EMU_LINUX_SIGSET_SIZE) & ~Emu_Linux_Unblockable();
    for (size_t i = 0; i < sizeof(Emu_x86_64_Linux_SigcontextRegs) / sizeof(Emu_x86_64_Linux_SigcontextRegs[0]); i++) {
        cpu->cs_reg[Emu_x86_64_Linux_SigcontextRegs[i]] = Emu_Get(sc + i * sizeof(uint64_t), sizeof(uint64_t));
    }
    cpu->cs_rip = Emu_Get(sc + EMU_X86_64_LINUX_SC_RIP_OFF, sizeof(uint64_t));
    Emu_x86_64_WriteFlags(cpu, Emu_Get(sc + EMU_X86_64_LINUX_SC_EFLAGS_OFF, sizeof(uint64_t)));
    if (area) {
        Emu_x86_64_RestoreFpu(cpu, area);
    } else {
        Emu_x86_64_ResetFpu(cpu);
    }
    return cpu->cs_reg[CPU_X86_64_REG_RAX];
}

// Deliver every pending signal the program does not block.
void Emu_x86_64_Linux_Deliver(Emu_x86_64_Linux_Guest *guest, Cpu_x86_64_State *cpu)
{
    Emu_Linux_Info kernel = {
        .ei_source = EMU_SOURCE_KERNEL,
        .ei_code   = EMU_LINUX_SI_KERNEL
    };

    while (! guest->eg_halted) {
        int32_t sig = Emu_Linux_NextSignal(guest);
        if (sig == EMU_LINUX_SIGNAL_NONE) {
            return;
        }

        // Phase: take one instance
        Emu_Linux_Info info = guest->eg_info[sig];
        Emu_Linux_Action action = guest->eg_action[sig];
        guest->eg_queued[sig]--;
        if (guest->eg_queued[sig] == 0) {
            guest->eg_pending &= ~Emu_Linux_SignalBit(sig);
        }

        // Phase: act on it
        if (action.ea_handler == EMU_LINUX_SIG_IGN || (action.ea_handler == EMU_LINUX_SIG_DFL && (Emu_Linux_IsDefaultIgnored(sig) || Emu_Linux_IsDefaultStop(sig)))) {
            continue;
        }
        if (action.ea_handler == EMU_LINUX_SIG_DFL) {
            if (info.ei_source == EMU_SOURCE_FAULT && cpu->cs_fault) {
                Log_Show(LOG_SEVERITY_ERROR, LOG_LINE_NONE, "%s", cpu->cs_fault);
            }
            guest->eg_halted = true;
            guest->eg_signal = sig;
            return;
        }
        if (action.ea_flags & EMU_X86_64_LINUX_SA_RESETHAND) {
            guest->eg_action[sig].ea_handler = EMU_LINUX_SIG_DFL;
        }
        if (! Emu_x86_64_Linux_PushFrame(guest, cpu, sig, &action, &info)) {
            if (sig == SIGSEGV) {
                guest->eg_action[sig].ea_handler = EMU_LINUX_SIG_DFL;
            }
            Emu_Linux_Force(guest, SIGSEGV, kernel);
        }
    }
}

// Turn the CPU's exception into the signal Linux sends for it.
void Emu_x86_64_Linux_Fault(Emu_x86_64_Linux_Guest *guest, Cpu_x86_64_State *cpu, uint64_t rip)
{
    Emu_Linux_Info info = {
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
            info.ei_code = EMU_LINUX_FPE_INTDIV;
        } break;
        case CPU_X86_64_VECTOR_UD: {
            sig = SIGILL;
            info.ei_code = EMU_LINUX_ILL_ILLOPN;
        } break;
        default: {
            guest->eg_err = cpu->cs_pf_err;
            info.ei_code = EMU_LINUX_SEGV_MAPERR;
            info.ei_addr = cpu->cs_cr2;
        }
    }
    Emu_Linux_Force(guest, sig, info);
}

// Answer a syscall as Linux does and fail the ones it lacks with ENOSYS.
void Emu_x86_64_Linux_Syscall(Emu_x86_64_Linux_Guest *guest, Cpu_x86_64_State *cpu)
{
    uint64_t *rax = &cpu->cs_reg[CPU_X86_64_REG_RAX];
    switch (*rax) {
        case EMU_X86_64_LINUX_SYSCALL_READ: {
            int32_t fd = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            uint64_t buf = cpu->cs_reg[CPU_X86_64_REG_RSI];
            uint64_t len = cpu->cs_reg[CPU_X86_64_REG_RDX];
            *rax = Emu_Linux_Read(guest, fd, buf, len);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_WRITE: {
            int32_t fd = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            uint64_t buf = cpu->cs_reg[CPU_X86_64_REG_RSI];
            uint64_t len = cpu->cs_reg[CPU_X86_64_REG_RDX];
            *rax = Emu_Linux_Write(guest, fd, buf, len);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_OPEN: {
            uint64_t addr = cpu->cs_reg[CPU_X86_64_REG_RDI];
            int32_t flags = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RSI];
            uint32_t mode = (uint32_t) cpu->cs_reg[CPU_X86_64_REG_RDX];
            *rax = Emu_Linux_Open(guest, addr, flags, mode);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_CLOSE: {
            int32_t fd = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            *rax = Emu_Linux_Result(close(fd));
        } break;
        case EMU_X86_64_LINUX_SYSCALL_LSEEK: {
            int32_t fd = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            int64_t off = (int64_t) cpu->cs_reg[CPU_X86_64_REG_RSI];
            int32_t whence = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDX];
            *rax = Emu_Linux_Result(lseek(fd, (off_t) off, whence));
        } break;
        case EMU_X86_64_LINUX_SYSCALL_BRK: {
            *rax = Emu_Linux_Brk(guest, cpu->cs_reg[CPU_X86_64_REG_RDI]);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_RT_SIGACTION: {
            int32_t sig = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            uint64_t act = cpu->cs_reg[CPU_X86_64_REG_RSI];
            uint64_t oact = cpu->cs_reg[CPU_X86_64_REG_RDX];
            uint64_t size = cpu->cs_reg[CPU_X86_64_REG_R10];
            *rax = Emu_x86_64_Linux_SigAction(guest, sig, act, oact, size);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_RT_SIGPROCMASK: {
            int32_t how = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            uint64_t set = cpu->cs_reg[CPU_X86_64_REG_RSI];
            uint64_t oset = cpu->cs_reg[CPU_X86_64_REG_RDX];
            uint64_t size = cpu->cs_reg[CPU_X86_64_REG_R10];
            *rax = Emu_Linux_SigProcMask(guest, how, set, oset, size);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_RT_SIGRETURN: {
            *rax = Emu_x86_64_Linux_SigReturn(guest, cpu);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_IOCTL: {
            int32_t fd = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            uint32_t req = (uint32_t) cpu->cs_reg[CPU_X86_64_REG_RSI];
            uint64_t arg = cpu->cs_reg[CPU_X86_64_REG_RDX];
            *rax = Emu_Linux_Ioctl(guest, fd, req, arg);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_DUP2: {
            int32_t old = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            int32_t new = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RSI];
            *rax = Emu_Linux_Result(dup2(old, new));
        } break;
        case EMU_X86_64_LINUX_SYSCALL_GETPID: {
            *rax = (uint64_t) getpid();
        } break;
        case EMU_X86_64_LINUX_SYSCALL_EXIT:
        case EMU_X86_64_LINUX_SYSCALL_EXIT_GROUP: {
            guest->eg_halted = true;
            guest->eg_status = cpu->cs_reg[CPU_X86_64_REG_RDI] & CPU_X86_64_MASK_8;
        } break;
        case EMU_X86_64_LINUX_SYSCALL_KILL: {
            int32_t pid = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            int32_t sig = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RSI];
            *rax = Emu_Linux_Kill(guest, pid, sig);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_RENAME: {
            uint64_t to = cpu->cs_reg[CPU_X86_64_REG_RSI];
            uint64_t from = cpu->cs_reg[CPU_X86_64_REG_RDI];
            *rax = Emu_Linux_Rename(guest, from, to);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_MKDIR: {
            uint64_t addr = cpu->cs_reg[CPU_X86_64_REG_RDI];
            uint32_t mode = (uint32_t) cpu->cs_reg[CPU_X86_64_REG_RSI];
            *rax = Emu_Linux_Mkdir(guest, addr, mode);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_RMDIR: {
            *rax = Emu_Linux_Rmdir(guest, cpu->cs_reg[CPU_X86_64_REG_RDI]);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_UNLINK: {
            *rax = Emu_Linux_Unlink(guest, cpu->cs_reg[CPU_X86_64_REG_RDI]);
        } break;
        case EMU_X86_64_LINUX_SYSCALL_CLOCK_GETTIME: {
            int32_t clock = (int32_t) cpu->cs_reg[CPU_X86_64_REG_RDI];
            uint64_t addr = cpu->cs_reg[CPU_X86_64_REG_RSI];
            *rax = Emu_Linux_ClockGettime(guest, clock, addr);
        } break;
        default: {
            *rax = -(uint64_t) EMU_LINUX_ERRNO_NOSYS;
        }
    }
}

// Run a loaded program to completion; return its status and ending signal.
int32_t Emu_x86_64_Linux_Run(Load_Image *img, Emu_Trace trace, int32_t *sig)
{
    Emu_x86_64_Linux_Guest guest = {
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
    Emu_Linux_Inherit(&guest);
    while (! guest.eg_halted) {
        uint64_t rip = cpu.cs_rip;

        if (trace == EMU_TRACE) {
            Emu_x86_64_ShowStep(&guest, rip);
        }
        Cpu_x86_64_Step(&cpu);
        switch (cpu.cs_trap) {
            case CPU_X86_64_TRAP_SYSCALL: {
                Emu_x86_64_Linux_Syscall(&guest, &cpu);
                Emu_x86_64_Linux_Deliver(&guest, &cpu);
            } break;
            case CPU_X86_64_TRAP_EXCEPTION: {
                Emu_x86_64_Linux_Fault(&guest, &cpu, rip);
                Emu_x86_64_Linux_Deliver(&guest, &cpu);
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
        Emu_x86_64_Disassemble(&img);
    } else {
        status = Emu_x86_64_Linux_Run(&img, trace, &sig);
    }

    Load_Free(&img);
    if (sig) {
        Emu_Raise(sig);
    }
    return status;
}
