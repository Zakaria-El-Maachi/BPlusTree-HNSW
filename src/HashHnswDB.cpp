#include "../include/HashHnswDB.h"

using namespace std;

// Constructor for HashHnswDB
HashHnswDB::HashHnswDB(int M, int ef_construction, int ef_search)
    : M(M), ef_construction(ef_construction), ef_search(ef_search) {}

// Build the hash-based index
void HashHnswDB::build_index(const vector<DataPoint>& data) {
    cout << "Data: " << data.size() << endl;
    // Group data points by category
    unordered_map<float, vector<DataPoint>> partitionedData;
    for (const auto& point : data) {
        partitionedData[point.category].push_back(point);
    }

    // Build an HNSW index for each category
    for (const auto& [category, points] : partitionedData) {
        cout << "Category: " << category << " size: " << points.size() << endl;
        HnswDB* hnsw = new HnswDB(M, ef_construction, ef_search, points.size());
        hnsw->build_index(points);
        categoryIndexMap[category] = hnsw;
    }
}

// Execute query on the corresponding HNSW index
vector<unsigned int> HashHnswDB::execute_query(const Query& query, unsigned int k, unsigned int expansionFactor) {
    // Find the HNSW index for the query's category
    auto it = categoryIndexMap.find(query.category_value);
    if (it == categoryIndexMap.end()) {
        throw runtime_error("No index found for the given category.");
    }

    // Delegate query execution to the corresponding HNSW index
    return it->second->execute_query(query, k, expansionFactor);
}
