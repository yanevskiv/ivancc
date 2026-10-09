/*
 * C header file for signal handling.
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

#ifndef _LIBC_IMPL_SIGNAL_H
#define _LIBC_IMPL_SIGNAL_H

// Signal handling
#define _LIBC_IMPL_SIGNAL_SIG_DFL ((void (*)(int)) 0)
#define _LIBC_IMPL_SIGNAL_SIG_ERR ((void (*)(int)) -1)
#define _LIBC_IMPL_SIGNAL_SIG_IGN ((void (*)(int)) 1)

#define _LIBC_IMPL_SIGNAL_SIGABRT 6
#define _LIBC_IMPL_SIGNAL_SIGFPE  8
#define _LIBC_IMPL_SIGNAL_SIGILL  4
#define _LIBC_IMPL_SIGNAL_SIGINT  2
#define _LIBC_IMPL_SIGNAL_SIGSEGV 11
#define _LIBC_IMPL_SIGNAL_SIGTERM 15

typedef int _Libc_Impl_Signal_sig_atomic_t;

// Specify signal handling
void (*_Libc_Impl_Signal_signal(int sig, void (*func)(int)))(int);

// Send signal
int _Libc_Impl_Signal_raise(int sig);

#endif // _LIBC_IMPL_SIGNAL_H
