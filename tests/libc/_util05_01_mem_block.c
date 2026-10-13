// (Test) Status: 0
// A test file for <_mem.h> blocks and counts.

// Includes.
#include <_test.h>
#include <stddef.h>
#include <_mem.h>

// A block's header should keep the memory after it aligned, counted in size_t.
static void Test_MemBlock(void)
{
    _Static_assert(sizeof(_Mem_Block) == _MEM_ALIGN, "a block's header is one alignment");
    _Static_assert(_MEM_BLOCK_MIN == 2 * _MEM_ALIGN, "the least block is two alignments");
    _Static_assert(sizeof(unsigned long) == sizeof(size_t), "_mem counts in a size_t");
}

int main(void)
{
    Test_MemBlock();
    return _Test_Status();
}
