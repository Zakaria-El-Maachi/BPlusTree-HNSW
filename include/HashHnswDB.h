#ifndef HASHHNSWDB_H
#define HASHHNSWDB_H

#include "../hnswlib/hnswlib/hnswlib.h"
#include "../hnswlib/hnswlib/space_l2.h"
#include "HnswDB.h"
#include <unordered_map>
#include <vector>
#include <array>

using namespace std;

// Hash-based HNSW Vector Database
class HashHnswDB : public VectorDatabase {
public:
    HashHnswDB(int M, int ef_construction, int ef_search);

    virtual void build_index(const vector<DataPoint>& data) override;
    virtual vector<unsigned int> execute_query(const Query& query, unsigned int k, unsigned int expansionFactor = 1) override;

private:
    unordered_map<float, HnswDB*> categoryIndexMap; // Maps category to its HNSW index
    int M, ef_construction, ef_search;
};

#endif // HASHHNSWDB_H