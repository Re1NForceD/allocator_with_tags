#include "block.hpp"
#include "tree.hpp"
#include <iostream>

block* block::split(size_t newSize)
{
    if ((this->getCurrentSize() - newSize) < (sizeof(struct block) + sizeof(struct Node))) return nullptr;

    block* newNextBlock = (block*)((char*)this + sizeof(struct block) + newSize);
    newNextBlock->setCurrentSize(this->getCurrentSize() - newSize - sizeof(struct block));
    newNextBlock->setPreviousSize(newSize);
    newNextBlock->setOffset(this->getOffset() + sizeof(struct block) + newSize);
    newNextBlock->copyFlags(this);
    newNextBlock->setBusy(false);
    newNextBlock->setFirst(false);
    if (block* nextNext = newNextBlock->next()) nextNext->setPreviousSize(newNextBlock->getCurrentSize());

    this->setCurrentSize(newSize);
    this->setLast(false);
    return newNextBlock;
}

block* block::merge()
{
    block* targetBlock = static_cast<block*>(this);

    block* prevBlock = targetBlock->prev();
    if(prevBlock && !prevBlock->isBusy())
    {
        treeRoot = deleteNode(treeRoot, (Node*)(prevBlock + 1));
        prevBlock->setCurrentSize(prevBlock->getCurrentSize() + targetBlock->getCurrentSize() + sizeof(struct block));
        prevBlock->mergeFlags(targetBlock);
        targetBlock = prevBlock;
    }

    block* nextBlock = targetBlock->next();
    if(nextBlock && !nextBlock->isBusy())
    {
        treeRoot = deleteNode(treeRoot, (Node*)(nextBlock + 1));
        targetBlock->setCurrentSize(targetBlock->getCurrentSize() + nextBlock->getCurrentSize() + sizeof(struct block));
        targetBlock->mergeFlags(nextBlock);
    }

    if (block* nextNext = targetBlock->next()) nextNext->setPreviousSize(targetBlock->getCurrentSize());
    return targetBlock;
}
