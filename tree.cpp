#include "tree.hpp"
#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

Node* treeRoot = nullptr;

int height(Node *N)
{
    if (!N) return 0;
    return N->height;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

Node *newNode(block* b)
{
    if (!b) return nullptr;
    Node* node = (Node*)(b + 1);
    // node->key = key;
    node->left = nullptr;
    node->right = nullptr;
    node->height = 1;
    node->nextSameKey = nullptr;
    return node;
}

Node *rightRotate(Node *y)
{
    Node *x = y->left;
    Node *T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;
    return x;
}

Node *leftRotate(Node *x)
{
    Node *y = x->right;
    Node *T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;
    return y;
}

int getBalanceFactor(Node *N)
{
    if (!N) return 0;
    return height(N->left) - height(N->right);
}

Node *insertNode(Node *node, Node* b)
{
    // cout << "key " << b->getKey() << " " << node << endl;
    if (!node) return b;
    if (!b) return node;

    if (b->getKey() < node->getKey())
    {
        // cout << "to left" << endl;
        Node* nn = insertNode(node->left, b);
        // if (nn == node->left) return node;
        node->left = nn;
        // cout << "set left " << nn << endl;
    }
    else if (b->getKey() > node->getKey())
    {
        // cout << "to right" << endl;
        Node* nn = insertNode(node->right, b);
        // if (nn == node->right) return node;
        node->right = nn;
        // cout << "set right " << nn << endl;
    }
    else
    {
        // cout << "sub node" << endl;
        return node->putSubNode(b->getBlock());
    }

    // cout << "balancing " << node << endl;
    node->height = 1 + max(height(node->left), height(node->right));
    int balanceFactor = getBalanceFactor(node);
    if (balanceFactor > 1) {
        if (b->getKey() < node->left->getKey()) return rightRotate(node);
        else if (b->getKey() > node->left->getKey())
        {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
    }
    if (balanceFactor < -1) {
        if (b->getKey() > node->right->getKey()) return leftRotate(node);
        else if (b->getKey() < node->right->getKey())
        {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
    }
    return node;
}

Node *nodeWithMimumValue(Node *node) {
    Node *current = node;
    while (current->left) current = current->left;
    return current;
}

Node *deleteNode(Node *root, Node *key, bool move)
{
    // cout << "del0 " << key << " " << key->getKey() << " " << root << " " << root->getKey() << endl;
    if (!root) return root;
    if (key->getKey() < root->getKey()) root->left = deleteNode(root->left, key);
    else if (key->getKey() > root->getKey()) root->right = deleteNode(root->right, key);
    else
    {
        // cout << "del00" << endl;
        if (root->nextSameKey && !move)
        {
            // cout << "del1 " << root->nextSameKey << " " << root->getKey() << endl;
            if (key == root)
            {
                // cout << "del4" << endl;
                Node* newNode = (Node*)root->nextSameKey;
                newNode->left = root->left;
                newNode->right = root->right;
                newNode->height = root->height;
                return newNode;
            }
            else
            {
                // cout << "del5" << endl;
                SubNode* prevNode = nullptr;
                SubNode* node = (SubNode*)root;
                while (node)
                {
                    if (node == key)
                    {
                        if (prevNode) prevNode->nextSameKey = node->nextSameKey;
                        return root;
                    }
                    prevNode = node;
                    node = node->nextSameKey;
                }
                throw string("No such node!");
            }
        }
        else if (!root->left || !root->right)
        {
            // cout << "del2 " << root->left << " " << root->right << " " << root->getKey() << endl;
            Node *temp = root->left ? root->left : root->right;
            if (!temp)
            {
                // cout << "del6" << endl;
                // temp = root;
                return nullptr;
            }
            else
            {
                // cout << "del7" << endl;
                return temp;
            }
            // free(temp);
        }
        else
        {
            // cout << "del3 " << root->getKey() << endl;
            Node *temp = nodeWithMimumValue(root->right);
            // cout << "del8 " << temp << " " << temp->getKey() << endl;
            // temp->getKey();
            temp->right = deleteNode(root->right, temp, true);
            temp->left = root->left;

            root = temp;
        }
    }

    if (!root) return root;

    root->height = 1 + max(height(root->left), height(root->right));
    int balanceFactor = getBalanceFactor(root);
    if (balanceFactor > 1)
    {
        if (getBalanceFactor(root->left) >= 0)
        {
            return rightRotate(root);
        }
        else
        {
            root->left = leftRotate(root->left);
            return rightRotate(root);
        }
    }
    if (balanceFactor < -1)
    {
        if (getBalanceFactor(root->right) <= 0) return leftRotate(root);
        else
        {
            root->right = rightRotate(root->right);
            return leftRotate(root);
        }
    }
    return root;
}

void printTree(Node* node, int i)
{
    if (node)
    {
        cout << setw(8) << node->getKey();
        if (SubNode* sn = node->nextSameKey)
        {
            size_t snA = 2;
            while (sn = sn->nextSameKey) ++snA;
            if (snA)
            {
                cout << endl;
                for (int j = 0; j < i * 2; ++j, cout << '\t');
                cout << "(" << setw(6) << snA << ')';
            }
        }

        if (node->right)
        {
            cout << " ------ ";
            printTree(node->right, i + 1);
        }
        if (node->left)
        {
            cout << endl;
            for (int j = 0; j < i * 2; ++j, cout << '\t');
            cout << "    |   "<< endl;
            for (int j = 0; j < i * 2; ++j, cout << '\t');
            printTree(node->left, i);
        }
    }
}
