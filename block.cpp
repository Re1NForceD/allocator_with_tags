#include "block.hpp"
#include <iostream>

void block::split(size_t newSize)
{
    if (this->sizeCurrent - newSize < sizeof(struct block)) return;

    block* newNextBlock = (block*)((char*)this + sizeof(struct block) + newSize);
    newNextBlock->sizeCurrent = this->sizeCurrent - newSize - sizeof(struct block);
    newNextBlock->sizePrevious = newSize;
    newNextBlock->busy = this->busy & ~(BUSY | FIRST);

    this->sizeCurrent = newSize;
    this->busy &= ~(BUSY | LAST);
}

void block::merge()
{
    block* nextBlock = this->next();
    if(!(nextBlock->busy & BUSY) && !(this->busy & BUSY))
    {
        this->sizeCurrent += nextBlock->sizeCurrent + sizeof(struct block);
        this->busy |= nextBlock->busy;
    }
}
