#ifndef HYBRID_DB_H
#define HYBRID_DB_H

#include <vector>
#include <functional>
#include "BPTreeDB.h"
#include "HnswDB.h"
#include "utils.h"

using namespace std;

// HybridDB class definition
class HybridDB : public VectorDatabase {
private:
    BPTreeDB bpTreeDb;
    HnswDB hnswDb;
    vector<DataPoint> data;

    // Helper method to calculate query length
    size_t calculate_query_length(const Query& query) const;

    // Deciding which index to forward the query to
    bool decide(const Query& query);

public:
    float alpha; // Threshold for query type 2

    // Constructor
    HybridDB(BPTreeDB bpt, HnswDB hnsw, float alphaVal);

    // Overridden methods from VectorDatabase
    void build_index(const vector<DataPoint>& data0) override;
    vector<unsigned int> execute_query(const Query& query, unsigned int k, unsigned int expansionFactor) override;
};

#endif // HYBRID_DB_H
