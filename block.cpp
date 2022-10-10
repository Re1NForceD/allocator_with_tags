#include "block.hpp"
#include "tree.hpp"
#include <iostream>

block* block::split(size_t newSize)
{
    if ((this->sizeCurrent - newSize) < (sizeof(struct block) + sizeof(struct Node))) return nullptr;

    block* newNextBlock = (block*)((char*)this + sizeof(struct block) + newSize);
    newNextBlock->sizeCurrent = this->sizeCurrent - newSize - sizeof(struct block);
    newNextBlock->sizePrevious = newSize;
    newNextBlock->busy = this->busy & ~(BUSY | FIRST);

    this->sizeCurrent = newSize;
    this->busy &= ~LAST;
    return newNextBlock;
}

block* block::merge()
{
    block* targetBlock = static_cast<block*>(this);

    block* prevBlock = targetBlock->prev();
    // std::cout << "try del prev " << std::endl;
    if(prevBlock && !prevBlock->isBusy()/* && !targetBlock->isBusy()*/)
    {
        // std::cout << "del prev " << prevBlock << " " << prevBlock->sizeCurrent << " " << prevBlock->isBusy() << std::endl;
        treeRoot = deleteNode(treeRoot, (Node*)(prevBlock + 1));
        // std::cout << "tree root " << treeRoot << std::endl;
        prevBlock->sizeCurrent += targetBlock->sizeCurrent + sizeof(struct block);
        prevBlock->busy |= targetBlock->busy;
        targetBlock = prevBlock;
    }

    // std::cout << "try del next " << std::endl;
    block* nextBlock = targetBlock->next();
    if(nextBlock && !nextBlock->isBusy()/* && !targetBlock->isBusy()*/)
    {
        // std::cout << "del mext " << nextBlock << " " << nextBlock->sizeCurrent << " " << nextBlock->isBusy() << std::endl;
        treeRoot = deleteNode(treeRoot, (Node*)(nextBlock + 1));
        // std::cout << "tree root " << treeRoot << std::endl;
        targetBlock->sizeCurrent += nextBlock->sizeCurrent + sizeof(struct block);
        targetBlock->busy |= nextBlock->busy;
    }

    if (block* nextNext = targetBlock->next()) nextNext->sizePrevious = targetBlock->sizeCurrent;
    return targetBlock;
}
