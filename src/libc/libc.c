/*
 * C source file for the freestanding libc.
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

// Print one byte to standard output.
int putchar(int c);

// Print the decimal digits of n, sign included, and return n.
int putd(int n)
{
    if (n < 0) {
        putchar('-');
        return putd(-n);
    }
    if (n >= 10) {
        putd(n / 10);
    }
    putchar(n % 10 + '0');
    return n;
}
