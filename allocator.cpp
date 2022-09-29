#include "allocator.hpp"
#include <windows.h>
#include <iostream>

#define PAGELIMIT 100
const size_t DEFAULT_ARENA = 50;

constexpr size_t align4(size_t x) { return (((((x) - 1) >> 2) << 2) + 4); }

static void* addressSpaceBegin = nullptr;
static void* nextAddress = nullptr;
static size_t pageSize = 0;

struct alignas(unsigned int) DataHeader {
    size_t size;
    int busy;
};

void* mem_alloc(size_t size)
{
    void* pointer = nullptr;
    auto s = align4(size) + sizeof(DataHeader);
    if (!addressSpaceBegin)
    {
        SYSTEM_INFO sSysInfo;
        GetSystemInfo(&sSysInfo);
        pageSize = sSysInfo.dwPageSize;
        std::cout << "First allocation! Page size: " << pageSize << ". Number of pages in arena: " << DEFAULT_ARENA << ". Header size: " << sizeof(DataHeader) << std::endl;
        addressSpaceBegin = VirtualAlloc(
                     NULL,
                     DEFAULT_ARENA * pageSize,
                     MEM_RESERVE | MEM_COMMIT,
                     PAGE_READWRITE);
        // DataHeader* header = (DataHeader*)addressSpaceBegin;
        //header->size =
        nextAddress = addressSpaceBegin;
    }
    std::cout << "SHISH!" << std::endl;
    pointer = nextAddress;
    DataHeader* header = (DataHeader*)pointer;
    header->size = align4(size);
    header->busy = 1;
    nextAddress = (void*)((char*)pointer + s);
    std::cout << s << " bytes allocated!" << std::endl;
    return (void*)((char*)pointer + sizeof(DataHeader));
}

void mem_free(void* ptr)
{
    if (ptr)
    {
        DataHeader* header = (DataHeader*)ptr - 1;
        std::cout << header->size + sizeof(DataHeader) << " bytes freed!" << std::endl;
        if (header->busy) {
            header->busy = 0;
        }
    }
}

void* mem_realloc(void* ptr, size_t size)
{
    return nullptr;
}

void mem_show()
{

}
