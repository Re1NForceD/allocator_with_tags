#include "kernel.hpp"
#include "config.hpp"
#include "block.hpp"
#include <windows.h>
#include <iostream>

void* kernel_alloc(size_t bytes)
{
    static size_t pageSize = -1; // TODO add allocated pages counter and use PAGE_LIMIT macros
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
        if (pages < DEFAULT_ARENA)
            pages = DEFAULT_ARENA;
    }

    std::cout << "Arena allocated! Number of " << (pageSize ? "pages" : "bytes") <<" in arena: " << pages << std::endl;
    block* arena = (block*)VirtualAlloc(NULL,
                                        pages * (pageSize ? pageSize : 1),
                                        MEM_RESERVE | MEM_COMMIT,
                                        PAGE_READWRITE);
    if (!arena)
        throw "Something went wrong!";

    arena->sizeCurrent = pages * (pageSize ? pageSize : 1) - sizeof(struct block);
    arena->sizePrevious = 0;
    arena->busy = FIRST | LAST;
    return (void*)arena;
}
