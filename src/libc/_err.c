/*
 * C source file for the texts a running program prints.
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
#include <_err.h>

// The error numbers the texts name.
#include <errno.h>

// The null pointer.
#include <stddef.h>

// Linux's error numbers C99 does not name.
#include <_sys.h>

// strerror's texts by error number, null for the numbers Linux leaves out.
const char *const _Err_ErrnoTexts[_ERR_ERRNO_COUNT] = {
    [_ERR_ERRNO_NONE]      = _ERR_ERRNO_SUCCESS,
    [_SYS_EPERM]           = _ERR_ERRNO_EPERM,
    [_SYS_ENOENT]          = _ERR_ERRNO_ENOENT,
    [_SYS_ESRCH]           = _ERR_ERRNO_ESRCH,
    [_SYS_EINTR]           = _ERR_ERRNO_EINTR,
    [_SYS_EIO]             = _ERR_ERRNO_EIO,
    [_SYS_ENXIO]           = _ERR_ERRNO_ENXIO,
    [_SYS_E2BIG]           = _ERR_ERRNO_E2BIG,
    [_SYS_ENOEXEC]         = _ERR_ERRNO_ENOEXEC,
    [_SYS_EBADF]           = _ERR_ERRNO_EBADF,
    [_SYS_ECHILD]          = _ERR_ERRNO_ECHILD,
    [_SYS_EAGAIN]          = _ERR_ERRNO_EAGAIN,
    [_SYS_ENOMEM]          = _ERR_ERRNO_ENOMEM,
    [_SYS_EACCES]          = _ERR_ERRNO_EACCES,
    [_SYS_EFAULT]          = _ERR_ERRNO_EFAULT,
    [_SYS_ENOTBLK]         = _ERR_ERRNO_ENOTBLK,
    [_SYS_EBUSY]           = _ERR_ERRNO_EBUSY,
    [_SYS_EEXIST]          = _ERR_ERRNO_EEXIST,
    [_SYS_EXDEV]           = _ERR_ERRNO_EXDEV,
    [_SYS_ENODEV]          = _ERR_ERRNO_ENODEV,
    [_SYS_ENOTDIR]         = _ERR_ERRNO_ENOTDIR,
    [_SYS_EISDIR]          = _ERR_ERRNO_EISDIR,
    [_SYS_EINVAL]          = _ERR_ERRNO_EINVAL,
    [_SYS_ENFILE]          = _ERR_ERRNO_ENFILE,
    [_SYS_EMFILE]          = _ERR_ERRNO_EMFILE,
    [_SYS_ENOTTY]          = _ERR_ERRNO_ENOTTY,
    [_SYS_ETXTBSY]         = _ERR_ERRNO_ETXTBSY,
    [_SYS_EFBIG]           = _ERR_ERRNO_EFBIG,
    [_SYS_ENOSPC]          = _ERR_ERRNO_ENOSPC,
    [_SYS_ESPIPE]          = _ERR_ERRNO_ESPIPE,
    [_SYS_EROFS]           = _ERR_ERRNO_EROFS,
    [_SYS_EMLINK]          = _ERR_ERRNO_EMLINK,
    [_SYS_EPIPE]           = _ERR_ERRNO_EPIPE,
    [EDOM]                 = _ERR_ERRNO_EDOM,
    [ERANGE]               = _ERR_ERRNO_ERANGE,
    [_SYS_EDEADLK]         = _ERR_ERRNO_EDEADLK,
    [_SYS_ENAMETOOLONG]    = _ERR_ERRNO_ENAMETOOLONG,
    [_SYS_ENOLCK]          = _ERR_ERRNO_ENOLCK,
    [_SYS_ENOSYS]          = _ERR_ERRNO_ENOSYS,
    [_SYS_ENOTEMPTY]       = _ERR_ERRNO_ENOTEMPTY,
    [_SYS_ELOOP]           = _ERR_ERRNO_ELOOP,
    [_SYS_ENOMSG]          = _ERR_ERRNO_ENOMSG,
    [_SYS_EIDRM]           = _ERR_ERRNO_EIDRM,
    [_SYS_ECHRNG]          = _ERR_ERRNO_ECHRNG,
    [_SYS_EL2NSYNC]        = _ERR_ERRNO_EL2NSYNC,
    [_SYS_EL3HLT]          = _ERR_ERRNO_EL3HLT,
    [_SYS_EL3RST]          = _ERR_ERRNO_EL3RST,
    [_SYS_ELNRNG]          = _ERR_ERRNO_ELNRNG,
    [_SYS_EUNATCH]         = _ERR_ERRNO_EUNATCH,
    [_SYS_ENOCSI]          = _ERR_ERRNO_ENOCSI,
    [_SYS_EL2HLT]          = _ERR_ERRNO_EL2HLT,
    [_SYS_EBADE]           = _ERR_ERRNO_EBADE,
    [_SYS_EBADR]           = _ERR_ERRNO_EBADR,
    [_SYS_EXFULL]          = _ERR_ERRNO_EXFULL,
    [_SYS_ENOANO]          = _ERR_ERRNO_ENOANO,
    [_SYS_EBADRQC]         = _ERR_ERRNO_EBADRQC,
    [_SYS_EBADSLT]         = _ERR_ERRNO_EBADSLT,
    [_SYS_EBFONT]          = _ERR_ERRNO_EBFONT,
    [_SYS_ENOSTR]          = _ERR_ERRNO_ENOSTR,
    [_SYS_ENODATA]         = _ERR_ERRNO_ENODATA,
    [_SYS_ETIME]           = _ERR_ERRNO_ETIME,
    [_SYS_ENOSR]           = _ERR_ERRNO_ENOSR,
    [_SYS_ENONET]          = _ERR_ERRNO_ENONET,
    [_SYS_ENOPKG]          = _ERR_ERRNO_ENOPKG,
    [_SYS_EREMOTE]         = _ERR_ERRNO_EREMOTE,
    [_SYS_ENOLINK]         = _ERR_ERRNO_ENOLINK,
    [_SYS_EADV]            = _ERR_ERRNO_EADV,
    [_SYS_ESRMNT]          = _ERR_ERRNO_ESRMNT,
    [_SYS_ECOMM]           = _ERR_ERRNO_ECOMM,
    [_SYS_EPROTO]          = _ERR_ERRNO_EPROTO,
    [_SYS_EMULTIHOP]       = _ERR_ERRNO_EMULTIHOP,
    [_SYS_EDOTDOT]         = _ERR_ERRNO_EDOTDOT,
    [_SYS_EBADMSG]         = _ERR_ERRNO_EBADMSG,
    [_SYS_EOVERFLOW]       = _ERR_ERRNO_EOVERFLOW,
    [_SYS_ENOTUNIQ]        = _ERR_ERRNO_ENOTUNIQ,
    [_SYS_EBADFD]          = _ERR_ERRNO_EBADFD,
    [_SYS_EREMCHG]         = _ERR_ERRNO_EREMCHG,
    [_SYS_ELIBACC]         = _ERR_ERRNO_ELIBACC,
    [_SYS_ELIBBAD]         = _ERR_ERRNO_ELIBBAD,
    [_SYS_ELIBSCN]         = _ERR_ERRNO_ELIBSCN,
    [_SYS_ELIBMAX]         = _ERR_ERRNO_ELIBMAX,
    [_SYS_ELIBEXEC]        = _ERR_ERRNO_ELIBEXEC,
    [EILSEQ]               = _ERR_ERRNO_EILSEQ,
    [_SYS_ERESTART]        = _ERR_ERRNO_ERESTART,
    [_SYS_ESTRPIPE]        = _ERR_ERRNO_ESTRPIPE,
    [_SYS_EUSERS]          = _ERR_ERRNO_EUSERS,
    [_SYS_ENOTSOCK]        = _ERR_ERRNO_ENOTSOCK,
    [_SYS_EDESTADDRREQ]    = _ERR_ERRNO_EDESTADDRREQ,
    [_SYS_EMSGSIZE]        = _ERR_ERRNO_EMSGSIZE,
    [_SYS_EPROTOTYPE]      = _ERR_ERRNO_EPROTOTYPE,
    [_SYS_ENOPROTOOPT]     = _ERR_ERRNO_ENOPROTOOPT,
    [_SYS_EPROTONOSUPPORT] = _ERR_ERRNO_EPROTONOSUPPORT,
    [_SYS_ESOCKTNOSUPPORT] = _ERR_ERRNO_ESOCKTNOSUPPORT,
    [_SYS_EOPNOTSUPP]      = _ERR_ERRNO_EOPNOTSUPP,
    [_SYS_EPFNOSUPPORT]    = _ERR_ERRNO_EPFNOSUPPORT,
    [_SYS_EAFNOSUPPORT]    = _ERR_ERRNO_EAFNOSUPPORT,
    [_SYS_EADDRINUSE]      = _ERR_ERRNO_EADDRINUSE,
    [_SYS_EADDRNOTAVAIL]   = _ERR_ERRNO_EADDRNOTAVAIL,
    [_SYS_ENETDOWN]        = _ERR_ERRNO_ENETDOWN,
    [_SYS_ENETUNREACH]     = _ERR_ERRNO_ENETUNREACH,
    [_SYS_ENETRESET]       = _ERR_ERRNO_ENETRESET,
    [_SYS_ECONNABORTED]    = _ERR_ERRNO_ECONNABORTED,
    [_SYS_ECONNRESET]      = _ERR_ERRNO_ECONNRESET,
    [_SYS_ENOBUFS]         = _ERR_ERRNO_ENOBUFS,
    [_SYS_EISCONN]         = _ERR_ERRNO_EISCONN,
    [_SYS_ENOTCONN]        = _ERR_ERRNO_ENOTCONN,
    [_SYS_ESHUTDOWN]       = _ERR_ERRNO_ESHUTDOWN,
    [_SYS_ETOOMANYREFS]    = _ERR_ERRNO_ETOOMANYREFS,
    [_SYS_ETIMEDOUT]       = _ERR_ERRNO_ETIMEDOUT,
    [_SYS_ECONNREFUSED]    = _ERR_ERRNO_ECONNREFUSED,
    [_SYS_EHOSTDOWN]       = _ERR_ERRNO_EHOSTDOWN,
    [_SYS_EHOSTUNREACH]    = _ERR_ERRNO_EHOSTUNREACH,
    [_SYS_EALREADY]        = _ERR_ERRNO_EALREADY,
    [_SYS_EINPROGRESS]     = _ERR_ERRNO_EINPROGRESS,
    [_SYS_ESTALE]          = _ERR_ERRNO_ESTALE,
    [_SYS_EUCLEAN]         = _ERR_ERRNO_EUCLEAN,
    [_SYS_ENOTNAM]         = _ERR_ERRNO_ENOTNAM,
    [_SYS_ENAVAIL]         = _ERR_ERRNO_ENAVAIL,
    [_SYS_EISNAM]          = _ERR_ERRNO_EISNAM,
    [_SYS_EREMOTEIO]       = _ERR_ERRNO_EREMOTEIO,
    [_SYS_EDQUOT]          = _ERR_ERRNO_EDQUOT,
    [_SYS_ENOMEDIUM]       = _ERR_ERRNO_ENOMEDIUM,
    [_SYS_EMEDIUMTYPE]     = _ERR_ERRNO_EMEDIUMTYPE,
    [_SYS_ECANCELED]       = _ERR_ERRNO_ECANCELED,
    [_SYS_ENOKEY]          = _ERR_ERRNO_ENOKEY,
    [_SYS_EKEYEXPIRED]     = _ERR_ERRNO_EKEYEXPIRED,
    [_SYS_EKEYREVOKED]     = _ERR_ERRNO_EKEYREVOKED,
    [_SYS_EKEYREJECTED]    = _ERR_ERRNO_EKEYREJECTED,
    [_SYS_EOWNERDEAD]      = _ERR_ERRNO_EOWNERDEAD,
    [_SYS_ENOTRECOVERABLE] = _ERR_ERRNO_ENOTRECOVERABLE,
    [_SYS_ERFKILL]         = _ERR_ERRNO_ERFKILL,
    [_SYS_EHWPOISON]       = _ERR_ERRNO_EHWPOISON
};

// Return strerror's text for the error number errnum, null for an unknown.
const char *_Err_ErrnoText(int errnum)
{
    if (errnum < 0 || errnum >= _ERR_ERRNO_COUNT) {
        return NULL;
    }
    return _Err_ErrnoTexts[errnum];
}
