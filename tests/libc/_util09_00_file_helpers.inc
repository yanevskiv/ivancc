// Includable helper file for <_file.h> tests.

// Includes.
#include <_test.h>
#include <_sys.h>
#include <_file.h>

// Make a stream on a new file that holds len bytes of data, set at its start.
static _File_Stream *opened(const char *data, unsigned long len)
{
    _File_Stream *stream = _File_MakeStream();

    _TEST_TRUE(stream != _FILE_NULL);
    _TEST_INT(_File_OpenFile(stream, "file", "w+"), 0);
    _TEST_UINT(_File_PutBytes(stream, data, len), len);
    _TEST_INT(_File_SeekStream(stream, 0, _SYS_SEEK_SET), 0);
    return stream;
}
