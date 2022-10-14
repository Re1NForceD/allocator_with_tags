#pragma once

#include "block.hpp"
#include <cstddef>
#include <iostream>

struct alignas(std::max_align_t) SubNode
{
    SubNode* nextSameKey = nullptr;
    inline block* getBlock()
    {
        return (block*)this - 1;
    }
    inline size_t getKey()
    {
        return getBlock()->getCurrentSize();
    };
};

struct alignas(std::max_align_t) Node: public SubNode
{
    Node* left;
    Node* right;
    size_t height;
    inline Node* putSubNode(block* b)
    {
        SubNode* subNode = (SubNode*)(b + 1);
        subNode->nextSameKey = nullptr;
        if (!nextSameKey) nextSameKey = subNode;
        else
        {
            SubNode* node = nextSameKey;
            while (node->nextSameKey) node = node->nextSameKey;
            node->nextSameKey = subNode;
        }
        return this;
    }
};

extern Node* treeRoot;

size_t max(size_t a, size_t b);
size_t height(Node *N);
Node* newNode(block*);
Node *rightRotate(Node *y);
Node *leftRotate(Node *x);
int getBalanceFactor(Node *N);
Node *insertNode(Node* node, Node* key);
Node *nodeWithMinValue(Node *node);
Node *deleteNode(Node *root, Node *key, bool move = false);
void printTree(Node *root, int =0);
