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
    _Static_assert(sizeof(_Scan_VaList) == sizeof(va_list), "_Scan_VaList should be va_list");
    _Static_assert(sizeof(_Scan_UintmaxType) == sizeof(uintmax_t) && (_Scan_UintmaxType) -1 > 0, "_Scan_UintmaxType should be uintmax_t");
    _Static_assert(sizeof(unsigned long) == sizeof(uintptr_t), "%p should read a pointer as an unsigned long");
    _Static_assert(sizeof(_Scan_WcharType) == sizeof(wchar_t), "_Scan_WcharType should be as wide as wchar_t");
    _Static_assert(((_Scan_WcharType) -1 < 0) == ((wchar_t) -1 < 0), "_Scan_WcharType should be signed as wchar_t");
    _Static_assert(_SCAN_INT_MAX == INT_MAX, "_SCAN_INT_MAX should be INT_MAX");
    _Static_assert(_SCAN_SIZE_MAX == SIZE_MAX, "_SCAN_SIZE_MAX should be SIZE_MAX");
    _Static_assert(_SCAN_BYTES_MAX == MB_LEN_MAX, "_SCAN_BYTES_MAX should be MB_LEN_MAX");
}

int main(void)
{
    Test_ScanLimits();
    return _Test_Status();
}
