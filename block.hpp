#pragma once
#include <cstddef>

struct alignas(std::max_align_t) block
{
    size_t sizeCurrent;
    size_t sizePrevious;
    size_t offset;

    block* merge();
    block* split(size_t);

    inline size_t getCurrentSize() { return sizeCurrent & ~1; };
    inline void setCurrentSize(size_t size) { this->sizeCurrent = size + isBusy(); };
    inline size_t getPreviousSize() { return sizePrevious & ~1; };
    inline void setPreviousSize(size_t size) { this->sizePrevious = size + isFirst(); };
    inline size_t getOffset() { return offset & ~1; };
    inline void setOffset(size_t offset) { this->offset = offset + isLast(); };

    inline block* next() { return isLast() ? nullptr : (block*)((char*)this + getCurrentSize() + sizeof(struct block)); };
    inline block* prev() { return isFirst() ? nullptr : (block*)((char*)this - getPreviousSize() - sizeof(struct block)); };

    inline void setBusy(bool flag) { sizeCurrent = (sizeCurrent & ~1) | (flag ? 1 : 0 ); };
    inline bool isBusy() { return sizeCurrent & 1; };
    inline void setFirst(bool flag) { sizePrevious = (sizePrevious & ~1) | (flag ? 1 : 0 ); };
    inline bool isFirst() { return sizePrevious & 1; };
    inline void setLast(bool flag) { offset = (offset & ~1) | (flag ? 1 : 0 ); };
    inline bool isLast() { return offset & 1; };

    inline void copyFlags(block* b)
    {
        this->setBusy(b->isBusy());
        this->setFirst(b->isFirst());
        this->setLast(b->isLast());
    }

    inline void mergeFlags(block* b)
    {
        this->setBusy(this->isBusy() | b->isBusy());
        this->setFirst(this->isFirst() | b->isFirst());
        this->setLast(this->isLast() | b->isLast());
    }
};
