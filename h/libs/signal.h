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

#ifndef __SIGNAL_H__
#define __SIGNAL_H__

// The implementation.
#include <libc/impl/libc_signal.h>

// (S7.14) Signal handling
#define SIG_DFL _LIBC_IMPL_SIGNAL_SIG_DFL
#define SIG_ERR _LIBC_IMPL_SIGNAL_SIG_ERR
#define SIG_IGN _LIBC_IMPL_SIGNAL_SIG_IGN

#define SIGABRT _LIBC_IMPL_SIGNAL_SIGABRT
#define SIGFPE  _LIBC_IMPL_SIGNAL_SIGFPE
#define SIGILL  _LIBC_IMPL_SIGNAL_SIGILL
#define SIGINT  _LIBC_IMPL_SIGNAL_SIGINT
#define SIGSEGV _LIBC_IMPL_SIGNAL_SIGSEGV
#define SIGTERM _LIBC_IMPL_SIGNAL_SIGTERM

typedef _Libc_Impl_Signal_sig_atomic_t sig_atomic_t;

// (S7.14.1) Specify signal handling
void (*signal(int sig, void (*func)(int)))(int);

// (S7.14.2) Send signal
int raise(int sig);

#endif // __SIGNAL_H__
