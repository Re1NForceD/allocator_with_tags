#pragma once
#include <cstddef>

#define BUSY  1
#define FIRST 2
#define LAST  4

struct alignas(std::max_align_t) block
{
    size_t sizeCurrent;
    size_t sizePrevious;
    char busy;
    block* merge();
    block* split(size_t);
    inline block* next() { return isLast() ? nullptr : (block*)((char*)this + sizeCurrent + sizeof(struct block)); };
    inline block* prev() { return isFirst() ? nullptr : (block*)((char*)this - sizePrevious - sizeof(struct block)); };
    inline bool isBusy() { return busy & BUSY; };
    inline bool isFirst() { return busy & FIRST; };
    inline bool isLast() { return busy & LAST; };
};
