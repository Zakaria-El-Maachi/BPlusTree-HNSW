#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include <vector>
#include <memory>
#include <algorithm>
#include <queue>
#include <functional>
#include <array>
#include <cassert>
#include "../include/utils.h"

using namespace std;

// B+ Tree Node Structure
class BPTreeNode {
public:
    static const int ORDER = 64; // Can be modified before compilation
    bool isLeaf;
    int numKeys;  // Number of keys in the node
    vector<float> keys;
    BPTreeNode* nextLeaf; // Pointer to the next leaf node

    // Union to save memory - either child pointers or data indices
    union NodePointers {
        vector<BPTreeNode*> childNodes; // For internal nodes
        vector<int> valueIndices;      // For leaf nodes

        NodePointers() {}
        ~NodePointers() {}
    } ptrs;

    BPTreeNode(bool leaf = false);
    ~BPTreeNode();
};

// BPlusTree class definition
class BPlusTree {
private:
    BPTreeNode* rootNode;

    // Helper function to find the parent node
    BPTreeNode* findParentNode(BPTreeNode* currentNode, BPTreeNode* targetChild);
    void insertInternalNode(float keyValue, BPTreeNode* parentNode, BPTreeNode* childNode);

public:
    BPlusTree();

    void insert(float keyValue, int dataIndex);
    BPTreeNode* searchFirstNode(float lowKey) const;

    friend class BPTreeDB;
};

// Vector Database Implementation
class BPTreeDB : public VectorDatabase {
private:
    vector<DataPoint> data;
    BPlusTree bptree;
    function<float(const array<float, 100>&, const array<float, 100>&)> distance_func;

public:
    BPTreeDB(function<float(const array<float, 100>&, const array<float, 100>&)> dist_func);

    virtual void build_index(const vector<DataPoint>& data);
    virtual vector<unsigned int> execute_query(const Query& query, unsigned int k, unsigned int expansionFactor = 1);
};

#endif
