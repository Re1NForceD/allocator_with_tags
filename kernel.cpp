#include "kernel.hpp"
#include "config.hpp"

#include <windows.h>
#include <iostream>
#include <string>

size_t pageSize = -1;
block* kernel_alloc(size_t bytes, bool default)
{
    if (pageSize == -1)
    {
        SYSTEM_INFO sSysInfo;
        GetSystemInfo(&sSysInfo);
        pageSize = sSysInfo.dwPageSize;

        std::cout << "Memory structure is ";
        if (pageSize)
            std::cout << "paged. Page size is " << pageSize << std::endl;
        else
            std::cout << "not paged" << std::endl;
        std::cout << "Header size is " << sizeof(struct block) << std::endl;
    }

    size_t pages = bytes;
    if (pageSize)
    {
        pages = bytes / pageSize + (bytes % pageSize ? 1 : 0);
        if (pages < DEFAULT_ARENA && default)
            pages = DEFAULT_ARENA;
    }

    std::cout << "Allocating arena... Number of " << (pageSize ? "pages" : "bytes") <<" in arena: " << pages << std::endl;
    block* arena = (block*)VirtualAlloc(nullptr,
                                        pages * (pageSize ? pageSize : 1),
                                        MEM_RESERVE | MEM_COMMIT,
                                        PAGE_READWRITE);
    if (!arena) // maximum possible 1934423032
        throw std::string("Error while allocating memory!");
    else
        std::cout << "Allocated arena " << arena << std::endl;

    arena->setCurrentSize(pages * (pageSize ? pageSize : 1) - sizeof(struct block));
    arena->setPreviousSize(0);
    arena->setOffset(sizeof(struct block));
    arena->setBusy(false);
    arena->setFirst(true);
    arena->setLast(true);
    return arena;
}

void kernel_free(void* arena)
{
    if (!VirtualFree(arena, 0, MEM_RELEASE))
        throw std::string("Error during freeing memory!");
}

void kernel_reset(void* ptr, size_t size)
{
    std::cout << "Inform kernel that memory " << ptr << " with size of " << size << " bytes do not used" << std::endl;
    VirtualAlloc(ptr,
                size,
                MEM_RESET,
                PAGE_READWRITE);
}
