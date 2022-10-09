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
    this->busy &= ~LAST;
}

void block::merge()
{
    block* targetBlock = static_cast<block*>(this);

    block* prevBlock = targetBlock->prev();
    if(prevBlock && !prevBlock->isBusy()/* && !targetBlock->isBusy()*/)
    {
        prevBlock->sizeCurrent += targetBlock->sizeCurrent + sizeof(struct block);
        prevBlock->busy |= targetBlock->busy;
        targetBlock = prevBlock;
    }

    block* nextBlock = targetBlock->next();
    if(nextBlock && !nextBlock->isBusy()/* && !targetBlock->isBusy()*/)
    {
        targetBlock->sizeCurrent += nextBlock->sizeCurrent + sizeof(struct block);
        targetBlock->busy |= nextBlock->busy;
    }

    if (block* nextNext = targetBlock->next()) nextNext->sizePrevious = targetBlock->sizeCurrent;
}
