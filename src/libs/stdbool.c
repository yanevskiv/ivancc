/*
 * C source file for the boolean type and values.
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
#include <stdbool.h>

// Check the header against the compiler.
typedef char _Libc_Stdbool_CheckSize[sizeof(bool) == sizeof(_Bool) ? 1 : -1];
typedef char _Libc_Stdbool_CheckTrue[true == 1 ? 1 : -1];
typedef char _Libc_Stdbool_CheckFalse[false == 0 ? 1 : -1];
typedef char _Libc_Stdbool_CheckDefined[__bool_true_false_are_defined == 1 ? 1 : -1];
typedef char _Libc_Stdbool_CheckConvert[(bool) 2 == 1 ? 1 : -1];
