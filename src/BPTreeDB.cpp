#include "../include/BPTreeDB.h"

using namespace std;


BPTreeNode::BPTreeNode(bool leaf) : isLeaf(leaf), numKeys(0), nextLeaf(nullptr) {
    keys.reserve(2 * ORDER);
    if (leaf) {
        new (&ptrs.valueIndices) vector<int>;
        ptrs.valueIndices.reserve(2 * ORDER);
    } else {
        new (&ptrs.childNodes) vector<BPTreeNode*>;
        ptrs.childNodes.reserve(2 * ORDER + 1);
    }
}


BPTreeNode::~BPTreeNode() {
    if (isLeaf) {
        ptrs.valueIndices.~vector<int>();
    } else {
        ptrs.childNodes.~vector<BPTreeNode*>();
    }
}


BPlusTree::BPlusTree(): rootNode(nullptr) {}


BPTreeNode* BPlusTree::findParentNode(BPTreeNode* currentNode, BPTreeNode* targetChild) {
    if (currentNode->isLeaf || currentNode->ptrs.childNodes.empty() || currentNode->ptrs.childNodes[0]->isLeaf)
        return nullptr;

    // Traverse the child nodes to locate the parent of targetChild
    for (int i = 0; i < currentNode->ptrs.childNodes.size(); i++) {
        if (currentNode->ptrs.childNodes[i] == targetChild) {
            return currentNode; // Return the parent node directly.
        } else {
            // Recursively search in the child nodes.
            BPTreeNode* result = findParentNode(currentNode->ptrs.childNodes[i], targetChild);
            if (result) {
                return result; // Return the parent node if found.
            }
        }
    }

    return nullptr; // Parent node not found.
}



// Inserts a key-value pair into the B+ tree
void BPlusTree::insert(float keyValue, int dataIndex) {
    if (rootNode == nullptr) {
        rootNode = new BPTreeNode(true);
        rootNode->keys.push_back(keyValue);
        rootNode->ptrs.valueIndices.push_back(dataIndex);
        rootNode->numKeys = 1;
        return;
    }

    BPTreeNode* currentNode = rootNode;
    BPTreeNode* parentNode = nullptr;

    // Traverse to the appropriate leaf node
    while (!currentNode->isLeaf) {
        parentNode = currentNode;
        auto insertPos = upper_bound(currentNode->keys.begin(), 
                                     currentNode->keys.begin() + currentNode->numKeys, 
                                     keyValue);
        int childIndex = insertPos - currentNode->keys.begin();
        currentNode = currentNode->ptrs.childNodes[childIndex];
    }

    // Insert into the leaf node if there's space
    if (currentNode->numKeys < 2 * BPTreeNode::ORDER) {
        auto insertPos = upper_bound(currentNode->keys.begin(), 
                                     currentNode->keys.begin() + currentNode->numKeys, 
                                     keyValue);
        int insertIndex = insertPos - currentNode->keys.begin();

        currentNode->keys.insert(currentNode->keys.begin() + insertIndex, keyValue);
        currentNode->ptrs.valueIndices.insert(currentNode->ptrs.valueIndices.begin() + insertIndex, dataIndex);
        currentNode->numKeys++;
    } else {
        // Split the leaf node if full
        BPTreeNode* newLeafNode = new BPTreeNode(true);

        vector<float> tempKeys(currentNode->keys);
        vector<int> tempValues(currentNode->ptrs.valueIndices);

        auto insertPos = upper_bound(tempKeys.begin(), tempKeys.end(), keyValue);
        int insertIndex = insertPos - tempKeys.begin();

        tempKeys.insert(tempKeys.begin() + insertIndex, keyValue);
        tempValues.insert(tempValues.begin() + insertIndex, dataIndex);

        int splitPoint = (2 * BPTreeNode::ORDER + 1) / 2;
        currentNode->numKeys = splitPoint;
        newLeafNode->numKeys = tempKeys.size() - splitPoint;

        currentNode->keys.assign(tempKeys.begin(), tempKeys.begin() + splitPoint);
        currentNode->ptrs.valueIndices.assign(tempValues.begin(), tempValues.begin() + splitPoint);

        newLeafNode->keys.assign(tempKeys.begin() + splitPoint, tempKeys.end());
        newLeafNode->ptrs.valueIndices.assign(tempValues.begin() + splitPoint, tempValues.end());

        newLeafNode->nextLeaf = currentNode->nextLeaf;
        currentNode->nextLeaf = newLeafNode;

        if (currentNode == rootNode) {
            BPTreeNode* newRootNode = new BPTreeNode(false);
            newRootNode->keys.push_back(newLeafNode->keys[0]);
            newRootNode->ptrs.childNodes.push_back(currentNode);
            newRootNode->ptrs.childNodes.push_back(newLeafNode);
            newRootNode->numKeys = 1;
            rootNode = newRootNode;
        } else {
            insertInternalNode(newLeafNode->keys[0], parentNode, newLeafNode);
        }
    }
}

void BPlusTree::insertInternalNode(float keyValue, BPTreeNode* parentNode, BPTreeNode* childNode) {
    if (parentNode->numKeys < 2 * BPTreeNode::ORDER) {
        auto insertPos = upper_bound(parentNode->keys.begin(), 
                                        parentNode->keys.begin() + parentNode->numKeys, 
                                        keyValue);
        int insertIndex = insertPos - parentNode->keys.begin();

        parentNode->keys.insert(parentNode->keys.begin() + insertIndex, keyValue);
        parentNode->ptrs.childNodes.insert(parentNode->ptrs.childNodes.begin() + insertIndex + 1, childNode);
        parentNode->numKeys++;
    } else {
        BPTreeNode* newInternalNode = new BPTreeNode(false);

        vector<float> tempKeys(parentNode->keys);
        vector<BPTreeNode*> tempChildren(parentNode->ptrs.childNodes);

        auto insertPos = upper_bound(tempKeys.begin(), tempKeys.end(), keyValue);
        int insertIndex = insertPos - tempKeys.begin();

        tempKeys.insert(tempKeys.begin() + insertIndex, keyValue);
        tempChildren.insert(tempChildren.begin() + insertIndex + 1, childNode);

        int middleIndex = BPTreeNode::ORDER;
        float middleKey = tempKeys[middleIndex];

        parentNode->numKeys = middleIndex;
        parentNode->keys.assign(tempKeys.begin(), tempKeys.begin() + middleIndex);
        parentNode->ptrs.childNodes.assign(tempChildren.begin(), tempChildren.begin() + middleIndex + 1);

        newInternalNode->keys.assign(tempKeys.begin() + middleIndex + 1, tempKeys.end());
        newInternalNode->ptrs.childNodes.assign(tempChildren.begin() + middleIndex + 1, tempChildren.end());
        newInternalNode->numKeys = tempKeys.size() - middleIndex - 1;

        if (parentNode == rootNode) {
            BPTreeNode* newRootNode = new BPTreeNode(false);
            newRootNode->keys.push_back(middleKey);
            newRootNode->ptrs.childNodes.push_back(parentNode);
            newRootNode->ptrs.childNodes.push_back(newInternalNode);
            newRootNode->numKeys = 1;
            rootNode = newRootNode;
        } else {
            BPTreeNode* grandparent = findParentNode(rootNode, parentNode);
            insertInternalNode(middleKey, grandparent, newInternalNode);
        }
    }
}

BPTreeNode* BPlusTree::searchFirstNode(float lowKey) const {
    if (!rootNode) return nullptr;

    BPTreeNode* currentNode = rootNode;

    // Traverse to the leftmost leaf within range
    while (!currentNode->isLeaf) {
        auto lowerPos = lower_bound(currentNode->keys.begin(), 
                                     currentNode->keys.begin() + currentNode->numKeys, 
                                     lowKey);
        int childIndex = lowerPos - currentNode->keys.begin();
        currentNode = currentNode->ptrs.childNodes[childIndex];
    }

    
    return currentNode;
}


BPTreeDB::BPTreeDB(function<float(const array<float, 100>&, const array<float, 100>&)> dist_func): distance_func(dist_func) {}


void BPTreeDB::build_index(const vector<DataPoint>& lkher) {
    data = lkher;
    delete bptree.rootNode;
    bptree.rootNode = nullptr;
    // float m = 1e9;
    // float ma = -1e9;
    for (size_t i = 0; i < data.size(); i++) {
        bptree.insert(data[i].timestamp, i);
        // ma = max(ma, data[i].timestamp);
        // m = min(m, data[i].timestamp);
    }
    // cout << "Max: " << ma << endl;
    // cout << "Min: " << m << endl;
}

vector<unsigned int> BPTreeDB::execute_query(const Query& query, unsigned int k, unsigned int expansionFactor) {
    // priority_queue<pair<float, unsigned int>, vector<pair<float, unsigned int>>, greater<pair<float, unsigned int>>> top_k;

    priority_queue<pair<float, unsigned int>> top_k;

    // cout << query.query_type << " " << query.category_value << " " << query.timestamp_start << "-" << query.timestamp_end << endl;

    if (query.query_type == 0 || query.query_type == 1) {
        // Full scan for query_type 0
        for (unsigned int i = 0; i < data.size(); ++i) {
            if(query.query_type == 1 && data[i].category != query.category_value) continue;
            float dist = distance_func(query.vector, data[i].vector);
            top_k.push({dist, i});
            if (top_k.size() > k) top_k.pop();
        }
    } else {
        
        BPTreeNode* node = bptree.searchFirstNode(query.timestamp_start);
        // for (int i = 0; i < node->numKeys; i++){
        //     cout << data[node->ptrs.valueIndices[i]].timestamp << ",";
        // }
        // cout << "]" << endl;
        // Collect all keys within the range
        bool shouldBreak = false;
        while (node) {
            if(shouldBreak) break;
            for (int i = 0; i < node->numKeys; i++) {
                if (node->keys[i] >= query.timestamp_start && node->keys[i] <= query.timestamp_end) {
                    int idx = node->ptrs.valueIndices[i];
                    bool include = true;
                    if(query.query_type == 3){
                        include = (data[idx].category == query.category_value);
                    }
                    if(!include) continue;
                    float dist = distance_func(query.vector, data[idx].vector);
                    top_k.push({dist, idx});
                    if (top_k.size() > k) top_k.pop();
                } else if(node->keys[i] > query.timestamp_end) {
                    shouldBreak = true;
                    break;
                }
            }
            node = node->nextLeaf;
        }

    }
    // if(top_k.empty()) cout << "Empty" << endl;

    // Convert priority queue to sorted vector (furthest to nearest)
    vector<unsigned int> result;
    result.reserve(top_k.size());
    while (!top_k.empty()) {
        result.push_back(top_k.top().second);
        top_k.pop();
    }
    return result;
}