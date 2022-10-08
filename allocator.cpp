#include "allocator.hpp"
#include <kernel.hpp>
#include <block.hpp>
#include <alignDefs.hpp>

#include <iostream>
#include <list>
#include <string>

std::list<block*> arenas;
block* findFit(size_t size);

void* mem_alloc(size_t size)
{
    std::cout << "Request for " << size << " bytes" << std::endl;
    auto alignSize = ROUND_BYTES(size);
    std::cout << "Align for " << ALIGN << "... Align size is " << alignSize << " bytes" << std::endl;
    if (alignSize < size) throw std::string("Size overflow while aligns!");

    block* header = (block*)findFit(alignSize);
    header->split(alignSize);
    header->busy |= BUSY;
    return (void*)((char*)header + sizeof(struct block));
}

block* findFit(size_t size)
{
    for (block* arena: arenas)
    {
        block* b = arena;
        while(true) {
            if (b->sizeCurrent > size)
                return b;

            if (b->busy & LAST) break;
            b = b->next();
        }
    }

    arenas.push_back(kernel_alloc(ROUND_BYTES(size) + sizeof(struct block)));
    return arenas.back();
}

void mem_free(void* ptr)
{
    block* header = (block*)((char*)ptr - sizeof(struct block));
    header->busy &= ~BUSY;
    header->merge();

    if ((!header->prev() && !header->isBusy() && !header->next()) || (header->prev() && !header->prev()->isBusy() && !header->prev()->next()))
    {
        auto arena = std::find(arenas.begin(), arenas.end(), header->prev() ? header->prev() : header);
        kernel_free(*arena);
        if (arena != arenas.end())
            arenas.erase(arena);
    }
}

void* mem_realloc(void* ptr, size_t size)
{
    return ptr;
}

void mem_show()
{
    int i = 0;
    for (block* arena: arenas)
    {
        size_t arenaSize = 0;
        std::cout << "Arena " << ++i << ":" << std::endl;
        int j = 0;
        block* b = arena;
        while(true) {
            std::cout << "\tBlock " << ++j << ":" << std::endl;
            std::cout << "\t\tprev size " << b->sizePrevious << std::endl;
            std::cout << "\t\tcurr size " << b->sizeCurrent << std::endl;
            std::cout << "\t\tflags: last->" << b->isLast() << " | first->" << b->isFirst() << " | busy->" << b->isBusy() << std::endl;

            arenaSize += sizeof(struct block) + b->sizeCurrent;
            if (b->busy & LAST) break;
            b = b->next();
        }
        std::cout << "\tTotal bytes: " << arenaSize << std::endl;
    }
    if (!arenas.size())
        std::cout << "No allocated memory with this allocator" << std::endl;
}

void freeAllMem() // for testing
{
    for (block* arena: arenas)
    {
        kernel_free(arena);
    }
    arenas.clear();
}
