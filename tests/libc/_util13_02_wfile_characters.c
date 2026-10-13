// (Test) Status: 0
// A test file for <_wfile.h> wide characters written and read.

// Include helpers.
#include "_util13_00_wfile_helpers.inc"

// A wide character should be written as its UTF-8 bytes and read back whole.
static void Test_WfileCharacters(void)
{
    _File_Stream *stream = _File_MakeStream();

    _Utf8_Enabled = 1;
    _TEST_TRUE(stream != _FILE_NULL);
    _TEST_INT(_File_OpenFile(stream, "file", "w+"), 0);
    _TEST_INT(stream->fs_orient, _FILE_ORIENT_NONE);
    _TEST_UINT(_Wfile_PutChar(stream, 'a'), 'a');
    _TEST_INT(stream->fs_orient, _FILE_ORIENT_WIDE);
    _TEST_UINT(_Wfile_PutChar(stream, EURO), EURO);
    _TEST_UINT(_Wfile_PutChar(stream, FACE), FACE);
    _TEST_INT(_File_SeekStream(stream, 1, _SYS_SEEK_SET), 0);
    _TEST_INT(_File_GetChar(stream), 0xE2);
    _TEST_INT(_File_GetChar(stream), 0x82);
    _TEST_INT(_File_GetChar(stream), 0xAC);
    _TEST_INT(_File_GetChar(stream), 0xF0);
    _TEST_INT(_File_SeekStream(stream, 0, _SYS_SEEK_SET), 0);
    _TEST_UINT(_Wfile_GetChar(stream), 'a');
    _TEST_UINT(_Wfile_GetChar(stream), EURO);
    _TEST_UINT(_Wfile_GetChar(stream), FACE);
    _TEST_UINT(_Wfile_GetChar(stream), _WFILE_END);
}

int main(void)
{
    Test_WfileCharacters();
    return _Test_Status();
}
