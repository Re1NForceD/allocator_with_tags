#include "allocator.hpp"
#include "kernel.hpp"
#include "block.hpp"
#include "tree.hpp"
#include "alignDefs.hpp"
#include "config.hpp"

#include <iostream>
#include <list>
#include <string>

struct ArenaList
{
    block* list;
    ArenaList() : list{ kernel_alloc(4096, false) }
    {
        std::cout << "For arenas " << list << " with size: " << list->sizeCurrent << " bytes" << std::endl;
        list->sizePrevious = 0;
    };
    ~ArenaList() {
        std::cout << "free all remain arenas" << std::endl;
        for (size_t i{ 0 }; i < getCount(); ++i)
            kernel_free(getListPtr()[i]);
        kernel_free(list);
    };
    bool addArena(block* ptr)
    {
        if (list->sizePrevious * sizeof(block*) > list->sizeCurrent - sizeof(block*)) return false;
        getListPtr()[getCount()] = ptr;
        list->sizePrevious++;
        return true;
    };
    bool removeArena(block* ptr)
    {
        if (!getCount()) return false;
        for (size_t i{ 0 }; i < getCount(); ++i)
        {
            block* b = getListPtr()[i];
            if (b == ptr)
            {
                if (i != getCount() - 1) memcpy(getListPtr() + i, getListPtr() + i + 1, (getCount() - i - 1) * sizeof(block*));
                list->sizePrevious--;
                kernel_free(ptr);
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
    if (alignSize < sizeof(struct Node)) alignSize = sizeof(struct Node);
    std::cout << "Align for " << ALIGN << "... Align size is " << alignSize << " bytes" << std::endl;
    if (alignSize < size) return nullptr; // overflow

    block* header;
    try
    {
        header = findFit(alignSize);
    }
    catch (...)
    {
        return nullptr; // too big memory request
    }

    if (alignSize < pageSize * DEFAULT_ARENA)
    {
        Node* node = newNode(header->split(alignSize));
        treeRoot = insertNode(treeRoot, node);
    }

    header->setBusy(true);
    return (void*)(header + 1);
}

Node* bestFit(Node* root, size_t key)
{
    if (!root) return nullptr;
    else if (root->getKey() == key) return root;
    else if (root->getKey() > key) {
        Node* node = bestFit(root->left, key);
        return node ? node : root;
    }
    else { // root->getKey() < key
        Node* node = bestFit(root->right, key);
        return node ? node : nullptr;
    }
}

block* findFit(size_t size)
{
    if (Node* node = bestFit(treeRoot, size))
    {
        treeRoot = deleteNode(treeRoot, node);
        return node->getBlock();
    }
    else
    {
        if (!arenas.addArena(kernel_alloc(size + sizeof(struct block)))) return nullptr; // out of space for arenas
        return arenas.getListPtr()[arenas.getCount() - 1];
    }
}

void mem_free(void* ptr)
{
    block* header = (block*)ptr - 1;
    header->setBusy(false);
    header = header->merge();

    if (!header->prev() && !header->next() && arenas.getCount() > 1)
    {
        arenas.removeArena(header);
    }
    else
    {
        if (header->getCurrentSize() >= pageSize)
        {
            auto leftOffset = pageSize - (header->getOffset() % pageSize);
            auto rightOffset = header->getCurrentSize() - (header->getCurrentSize() % pageSize);

            if (leftOffset < rightOffset) kernel_reset((char*)(header + 1) + leftOffset, rightOffset - leftOffset);
        }
        treeRoot = insertNode(treeRoot, newNode(header));
    }
}

void* mem_realloc(void* ptr, size_t size)
{
    auto alignSize = ROUND_BYTES(size) < sizeof(struct Node) ? sizeof(struct Node) : ROUND_BYTES(size);
    block* b = (block*)ptr - 1;
    size_t currSize = b->getCurrentSize();
    if (currSize == alignSize) return ptr;

    if (currSize >= pageSize * DEFAULT_ARENA)
    {
        if (currSize >> 3 >= (alignSize > currSize ? alignSize - currSize : currSize - alignSize)) return ptr;
        else
        {
            block* newBlock = (block*)mem_alloc(alignSize) - 1;
            if (!newBlock) return nullptr;

            memcpy(newBlock + 1, ptr, (alignSize > currSize ? currSize : alignSize));

            b->setBusy(false);
            arenas.removeArena(b);

            return (void*)(newBlock + 1);
        }
    }
    else
    {
        if (alignSize < currSize) // in-place
        {
            block* fromSplit = b->split(alignSize);
            if (fromSplit)
            {
                fromSplit = fromSplit->merge();
                treeRoot = insertNode(treeRoot, newNode(fromSplit));
            }
            return ptr;
        }
        else
        {
            if (!b->next()->isBusy() &&
                (!b->prev() || b->prev()->isBusy()) &&
                (b->next()->getCurrentSize() + sizeof(struct block) + b->getCurrentSize() >= alignSize)) // increase size in-place
            {
                if (block* fromSplit = b->merge()->split(alignSize))
                {
                    treeRoot = insertNode(treeRoot, newNode(fromSplit));
                }
                return ptr;
            }
            else // increase size new place
            {
                block* newBlock = (block*)mem_alloc(alignSize) - 1;
                if (!newBlock) return nullptr;

                memcpy(newBlock + 1, ptr, currSize);

                b->setBusy(false);
                treeRoot = insertNode(treeRoot, newNode(b->merge()));

                return (void*)(newBlock + 1);
            }
        }
    }
}

void mem_show()
{
    if (treeRoot)
    {
        std::cout << "AVL Tree:" << std::endl;
        printTree(treeRoot);
        std::cout << std::endl;
    }
    else std::cout << "AVL Tree is empty" << std::endl;

    for (size_t i{ 0 }; i < arenas.getCount(); ++i)
    {
        size_t arenaSize = 0;
        std::cout << "Arena " << i + 1 << "/" << arenas.getCount() << ": " << arenas.getListPtr()[i] << std::endl;
        int j = 0;
        block* b = arenas.getListPtr()[i];
        while(true) {
            std::cout << "\tBlock " << ++j << ": " << b << std::endl;
            std::cout << "\t\tprev size " << b->getPreviousSize() << std::endl;
            std::cout << "\t\tcurr size " << b->getCurrentSize() << std::endl;
            std::cout << "\t\tflags: last->" << b->isLast() << " | first->" << b->isFirst() << " | busy->" << b->isBusy() << " | offset: " << b->getOffset() << std::endl;

            arenaSize += sizeof(struct block) + b->getCurrentSize();
            if (b->isLast()) break;
            b = b->next();
        }
        std::cout << "\tTotal bytes: " << arenaSize << std::endl;
    }
    if (!arenas.getCount())
        std::cout << "No allocated memory with this allocator" << std::endl;
}
