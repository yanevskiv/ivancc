/*
 * C header file for the system layer.
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

#ifndef __SYS_SYS_H__
#define __SYS_SYS_H__

// System calls
long __libc_syscall(long number, long arg1, long arg2, long arg3, long arg4, long arg5, long arg6);
long __libc_write(int fd, const void *buf, unsigned long len);
void __libc_exit(int status);

#endif // __SYS_SYS_H__
