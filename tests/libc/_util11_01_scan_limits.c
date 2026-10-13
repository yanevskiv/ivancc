// (Test) Status: 0
// A test file for <_scan.h> types and limits against C99's.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <stdarg.h>
#include <_scan.h>

// _scan's types and limits should be C99's.
static void Test_ScanLimits(void)
{
    _Static_assert(sizeof(_Scan_VaList) == sizeof(va_list), "_Scan_VaList is va_list");
    _Static_assert(sizeof(_Scan_UintmaxType) == sizeof(uintmax_t) && (_Scan_UintmaxType) -1 > 0, "_Scan_UintmaxType is uintmax_t");
    _Static_assert(sizeof(unsigned long) == sizeof(uintptr_t), "%p reads a pointer as an unsigned long");
    _Static_assert(sizeof(_Scan_WcharType) == sizeof(wchar_t), "_Scan_WcharType is as wide as wchar_t");
    _Static_assert(((_Scan_WcharType) -1 < 0) == ((wchar_t) -1 < 0), "_Scan_WcharType is signed as wchar_t");
    _Static_assert(_SCAN_INT_MAX == INT_MAX, "_SCAN_INT_MAX is INT_MAX");
    _Static_assert(_SCAN_SIZE_MAX == SIZE_MAX, "_SCAN_SIZE_MAX is SIZE_MAX");
    _Static_assert(_SCAN_BYTES_MAX == MB_LEN_MAX, "_SCAN_BYTES_MAX is MB_LEN_MAX");
}

int main(void)
{
    Test_ScanLimits();
    return _Test_Status();
}
