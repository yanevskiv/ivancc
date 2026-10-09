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

#ifndef __LIBC_IMPL_SIGNAL_H__
#define __LIBC_IMPL_SIGNAL_H__

// The handlers and signals the functions take.
#include <signal.h>

// (S7.14.1) Specify signal handling
void (*__libc_impl_signal_signal(int sig, void (*func)(int)))(int);

// (S7.14.2) Send signal
int __libc_impl_signal_raise(int sig);

#endif // __LIBC_IMPL_SIGNAL_H__
