// (Test) Status: 0
// A test file for <_wfile.h> wide characters pushed back.

// Include helpers.
#include "_util13_00_wfile_helpers.inc"

// A wide character should go back as its bytes, whole or not at all.
static void Test_WfilePushback(void)
{
    _File_Stream *stream = opened("b", 1);

    _TEST_UINT(_Wfile_UngetChar(stream, _WFILE_END), _WFILE_END);
    _TEST_UINT(stream->fs_backs, 0);
    _TEST_UINT(_Wfile_UngetChar(stream, EURO), EURO);
    _TEST_UINT(stream->fs_backs, 3);
    _TEST_UINT(_Wfile_UngetChar(stream, EURO), _WFILE_END);
    _TEST_UINT(stream->fs_backs, 3);
    _TEST_UINT(_Wfile_UngetChar(stream, 'x'), 'x');
    _TEST_UINT(_Wfile_UngetChar(stream, 'y'), _WFILE_END);
    _TEST_UINT(_Wfile_GetChar(stream), 'x');
    _TEST_UINT(_Wfile_GetChar(stream), EURO);
    _TEST_UINT(_Wfile_GetChar(stream), 'b');
    _TEST_UINT(_Wfile_GetChar(stream), _WFILE_END);
    _TEST_UINT(_Wfile_UngetChar(stream, FACE), FACE);
    _TEST_UINT(stream->fs_backs, _FILE_BACKS);
    _TEST_UINT(_Wfile_GetChar(stream), FACE);
}

int main(void)
{
    Test_WfilePushback();
    return _Test_Status();
}
