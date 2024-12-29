#ifndef HYBRID_HASH_DB_H
#define HYBRID_HASH_DB_H

#include <vector>
#include <functional>
#include "BPTreeDB.h"
#include "HnswDB.h"
#include "HashHnswDB.h"
#include "utils.h"

using namespace std;

// HybridHashDB class definition
class HybridHashDB : public VectorDatabase {
private:
    BPTreeDB bpTreeDb;
    HnswDB hnswDb;
    HashHnswDB hashHnswDb;
    vector<DataPoint> data;

    // Helper method to calculate query length
    size_t calculate_query_length(const Query& query) const;

    // Deciding which index to forward the query to
    bool decide(const Query& query);

public:

    float alpha; // Threshold for query type 2
    float beta;  // Threshold for query type 3

    // Constructor
    HybridHashDB(BPTreeDB bpt, HnswDB hnsw, HashHnswDB hashHnsw, float alphaVal, float betaVal);

    // Overridden methods from VectorDatabase
    void build_index(const vector<DataPoint>& data0) override;
    vector<unsigned int> execute_query(const Query& query, unsigned int k, unsigned int expansionFactor) override;
};

#endif // HYBRID_HASH_DB_H
