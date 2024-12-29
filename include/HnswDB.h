#ifndef HNSWDB_H
#define HNSWDB_H

#include "../hnswlib/hnswlib/hnswlib.h"
#include "../hnswlib/hnswlib/space_l2.h"
#include <vector>
#include <array>
#include "../include/utils.h"

using namespace std;

// HNSW-based Vector Database
class HnswDB : public VectorDatabase {
public:
    HnswDB(int M, int ef_construction, int ef_search, int max_elements);

    virtual void build_index(const vector<DataPoint>& data);
    virtual vector<unsigned int> execute_query(const Query& query, unsigned int k, unsigned int expansionFactor = 1);

private:
    hnswlib::L2Space* space;
    hnswlib::HierarchicalNSW<float>* index;
    vector<DataPoint> data;
    int ef_search;

    // Check if a DataPoint matches query constraints
    bool matches_constraints(const Query& query, const DataPoint& point);
};

#endif // HNSWDB_H
