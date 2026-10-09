/*
 * C source file for signal handling.
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
 * Under Section 7 of GPL version 3, you are granted additional
 * permissions described in the GCC Runtime Library Exception, version
 * 3.1, as published by the Free Software Foundation.
 *
 * You should have received a copy of the GNU General Public License and
 * a copy of the GCC Runtime Library Exception along with ivancc; see
 * the files LICENSE and COPYING.RUNTIME respectively.  If not, see
 * <https://www.gnu.org/licenses/>.
 */

// Module header.
#include <libc/impl/libc_signal.h>

// The error number a refused request sets.
#include <libc/impl/libc_errno.h>

// The system calls that install a handler and send a signal.
#include <libc/libc_sys.h>

// Install func as the handler of the signal sig.
void (*_Libc_Impl_Signal_signal(int sig, void (*func)(int)))(int)
{
    struct _Libc_Sys_sigaction act;
    struct _Libc_Sys_sigaction old;
    long ret;

    if (sig < _LIBC_SYS_SIGNAL_FIRST || sig > _LIBC_SYS_SIGNAL_LAST || func == _LIBC_IMPL_SIGNAL_SIG_ERR) {
        errno = _LIBC_SYS_EINVAL;
        return _LIBC_IMPL_SIGNAL_SIG_ERR;
    }
    act.sa_handler = func;
    act.sa_flags = _LIBC_SYS_SA_RESTORER | _LIBC_SYS_SA_RESTART;
    act.sa_restorer = _Libc_Sys_restore_rt;
    act.sa_mask = 1UL << (sig - _LIBC_SYS_SIGNAL_FIRST);
    ret = _Libc_Sys_rt_sigaction(sig, &act, &old);
    if (ret < 0) {
        errno = (int) -ret;
        return _LIBC_IMPL_SIGNAL_SIG_ERR;
    }
    return old.sa_handler;
}

// Send the signal sig to the program.
int _Libc_Impl_Signal_raise(int sig)
{
    long ret = _Libc_Sys_kill(_Libc_Sys_getpid(), sig);

    if (ret < 0) {
        errno = (int) -ret;
        return -1;
    }
    return 0;
}
