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
#include <ivancc/impl/libc_signal.h>

// The error number a refused request sets.
#include <errno.h>

// The system calls that install a handler and send a signal.
#include <ivancc/libc_sys.h>

// Install func as the handler of the signal sig.
void (*__libc_impl_signal_signal(int sig, void (*func)(int)))(int)
{
    struct __libc_sys_sigaction act;
    struct __libc_sys_sigaction old;
    long ret;

    if (sig < __LIBC_SYS_SIGNAL_FIRST || sig > __LIBC_SYS_SIGNAL_LAST || func == SIG_ERR) {
        errno = __LIBC_SYS_EINVAL;
        return SIG_ERR;
    }
    act.sa_handler = func;
    act.sa_flags = __LIBC_SYS_SA_RESTORER | __LIBC_SYS_SA_RESTART;
    act.sa_restorer = __libc_sys_restore_rt;
    act.sa_mask = 1UL << (sig - __LIBC_SYS_SIGNAL_FIRST);
    ret = __libc_sys_rt_sigaction(sig, &act, &old);
    if (ret < 0) {
        errno = (int) -ret;
        return SIG_ERR;
    }
    return old.sa_handler;
}

// Send the signal sig to the program.
int __libc_impl_signal_raise(int sig)
{
    long ret = __libc_sys_kill(__libc_sys_getpid(), sig);

    if (ret < 0) {
        errno = (int) -ret;
        return -1;
    }
    return 0;
}
