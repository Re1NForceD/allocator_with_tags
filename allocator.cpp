#include "allocator.hpp"
#include <kernel.hpp>
#include <block.hpp>
#include <alignDefs.hpp>

#include <iostream>
#include <list>
#include <string>

struct ArenaList
{
    block* list;
    ArenaList() : list{ kernel_alloc(1, false) }
    {
        list->sizePrevious = 0;
    };
    ~ArenaList() {
        kernel_free(list);
    };
    bool addArena(block* ptr)
    {
        if (list->sizePrevious * sizeof(block*) > list->sizeCurrent - sizeof(block*)) return false;
        block** b = getListPtr() + getCount();
        *b = ptr;
        list->sizePrevious++;
        return true;
    };
    bool removeArena(block* ptr)
    {
        if (!getCount()) return false;
        for (auto i{ 0 }; i < getCount(); ++i)
        {
            block* b = *getListPtr() + i;
            if (b == ptr)
            {
                if (i != getCount() - 1) memcpy(ptr, ptr + 1, (getCount() - i - 1) * sizeof(block*));
                list->sizePrevious--;
                return true;
            }
        }
         return false;
    };
    inline block** getListPtr()
    {
        return (block**)(list + 1);
    };
    inline size_t getCount()
    {
        return list->sizePrevious;
    };
};

static ArenaList arenas;
block* findFit(size_t size);

void* mem_alloc(size_t size)
{
    std::cout << "Request for " << size << " bytes" << std::endl;
    auto alignSize = ROUND_BYTES(size);
    std::cout << "Align for " << ALIGN << "... Align size is " << alignSize << " bytes" << std::endl;
    if (alignSize < size) throw std::string("Size overflow while aligns!");

    block* header = findFit(alignSize);
    header->split(alignSize);
    header->busy |= BUSY;
    return (void*)(header + 1);
}

block* findFit(size_t size)
{
    for (auto i{ 0 }; i < arenas.getCount(); ++i)
    {
        block* b = *arenas.getListPtr() + i;
        while(true) {
            if (b->sizeCurrent > size)
                return b;

            if (b->busy & LAST) break;
            b = b->next();
        }
    }

    arenas.addArena(kernel_alloc(ROUND_BYTES(size) + sizeof(struct block)));
    return *arenas.getListPtr() + (arenas.getCount() - 1);
}

void mem_free(void* ptr)
{
    block* header = (block*)((char*)ptr - sizeof(struct block));
    header->busy &= ~BUSY;
    header->merge();

    if ((!header->prev() && !header->isBusy() && !header->next()) || (header->prev() && !header->prev()->isBusy() && !header->prev()->next()))
    {
        arenas.removeArena(header->prev() ? header->prev() : header);
        kernel_free(header->prev() ? header->prev() : header);
    }
}

void* mem_realloc(void* ptr, size_t size)
{
    block* b = (block*)((char*)ptr - sizeof(struct block));
    auto alignSize = ROUND_BYTES(size);
    if (size <= b->sizeCurrent)
    {
        b->split(alignSize);
        b->next()->merge();
        return ptr;
    }
    else
    {
        if (!b->next()->isBusy() && (b->next()->sizeCurrent + b->sizeCurrent >= alignSize))
        {
            b->merge();
            b->split(alignSize);
            return ptr;
        }
        else
        {
            block* newBlock = findFit(alignSize);
            newBlock->split(alignSize);
            newBlock->busy |= BUSY;
            newBlock->next()->merge();
            memcpy(newBlock + 1, ptr, b->sizeCurrent);
            mem_free(ptr);
            return (void*)(newBlock + 1);
        }
    }
}

void mem_show()
{
    for (auto i{ 0 }; i < arenas.getCount(); ++i)
    {
        size_t arenaSize = 0;
        std::cout << "Arena " << ++i << ":" << std::endl;
        int j = 0;
        block* b = *arenas.getListPtr() + i - 1;
        while(true) {
            std::cout << "\tBlock " << ++j << ": " << b << std::endl;
            std::cout << "\t\tprev size " << b->sizePrevious << std::endl;
            std::cout << "\t\tcurr size " << b->sizeCurrent << std::endl;
            std::cout << "\t\tflags: last->" << b->isLast() << " | first->" << b->isFirst() << " | busy->" << b->isBusy() << std::endl;

            arenaSize += sizeof(struct block) + b->sizeCurrent;
            if (b->busy & LAST) break;
            b = b->next();
        }
        std::cout << "\tTotal bytes: " << arenaSize << std::endl;
    }
    if (!arenas.getCount())
        std::cout << "No allocated memory with this allocator" << std::endl;
}
