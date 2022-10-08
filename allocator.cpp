#include "allocator.hpp"
#include <kernel.hpp>
#include <block.hpp>
#include <iostream>
#include <vector>
#include <string>

#define ALIGN alignof(std::max_align_t)
#define ROUND_BYTES(s) (((s) + (ALIGN - 1)) & ~(ALIGN - 1))

std::vector<void*> arenas;
block* findFit(size_t size);

void* mem_alloc(size_t size)
{
    std::cout << "Request for " << size << " bytes" << std::endl;
    auto alignSize = ROUND_BYTES(size);
    std::cout << "Align for " << ALIGN << "... Align size is " << alignSize << " bytes" << std::endl;
    if (alignSize < size) throw std::string("Size overflow while aligns!");
    alignSize += sizeof(struct block);

    block* header = (block*)findFit(alignSize);
    header->split(alignSize);
    header->busy |= BUSY;
    return (void*)((char*)header + sizeof(struct block));
}

block* findFit(size_t size)
{
    for (const void* arena: arenas)
    {
        block* b = (block*)arena;
        while(true) {
            if (b->sizeCurrent > size)
                return b;

            if (b->busy & LAST) break;
            b = b->next();
        }
    }

    arenas.push_back(kernel_alloc(ROUND_BYTES(size)));
    return (block*)(arenas[arenas.size() - 1]);
}

void mem_free(void* ptr)
{
    block* header = (block*)((char*)ptr - sizeof(struct block));
    header->busy &= ~BUSY;
    header->merge();
}

void* mem_realloc(void* ptr, size_t size)
{
    return ptr;
}

void mem_show()
{
    int i = 0;
    for (const void* arena: arenas)
    {
        size_t arenaSize = 0;
        std::cout << "Arena " << ++i << ":" << std::endl;
        int j = 0;
        block* b = (block*)arena;
        while(true) {
            std::cout << "\tBlock " << ++j << ":" << std::endl;
            std::cout << "\t\tcurr size " << b->sizeCurrent << std::endl;
            std::cout << "\t\tprev size " << b->sizePrevious << std::endl;
            std::cout << "\t\tflags " << !!(b->busy & LAST) << " " << !!(b->busy & FIRST) << " " << !!(b->busy & BUSY) << std::endl;

            arenaSize += sizeof(struct block) + b->sizeCurrent;
            if (b->busy & LAST) break;
            b = b->next();
        }
        std::cout << "\tTotal bytes: " << arenaSize << std::endl;
    }
    if (!arenas.size())
        std::cout << "No allocated memory with this allocator" << std::endl;
}
