// (Test) Status: 0
// A test file for <_wfile.h> bytes that are no character.

// Include helpers.
#include "_util13_00_wfile_helpers.inc"

// Bytes that are no character should fail a read with EILSEQ, an end with none.
static void Test_WfileInvalid(void)
{
    _File_Stream *stream = opened("\xE2\x82" "a", 3);
    int wc = 0;

    _Err_Errno = 0;
    _TEST_INT(_Wfile_ReadChar(stream, &wc), _WFILE_STATUS_FAILED);
    _TEST_INT(_Err_Errno, _SYS_EILSEQ);
    _TEST_TRUE((stream->fs_flags & _FILE_ERROR) != 0);
    stream = opened("a\xE2\x82", 3);
    _Err_Errno = 0;
    _TEST_INT(_Wfile_ReadChar(stream, &wc), _WFILE_STATUS_GOOD);
    _TEST_INT(wc, 'a');
    _TEST_INT(_Wfile_ReadChar(stream, &wc), _WFILE_STATUS_FAILED);
    _TEST_INT(_Err_Errno, _SYS_EILSEQ);
    stream = opened("", 0);
    _Err_Errno = 0;
    _TEST_INT(_Wfile_ReadChar(stream, &wc), _WFILE_STATUS_END);
    _TEST_INT(_Err_Errno, 0);
    _TEST_TRUE((stream->fs_flags & _FILE_ERROR) == 0);
}

int main(void)
{
    Test_WfileInvalid();
    return _Test_Status();
}
