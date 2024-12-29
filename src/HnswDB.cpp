#include "../include/HnswDB.h"

// Constructor for the HNSW database
HnswDB::HnswDB(int M, int ef_construction, int ef_search, int max_elements)
    : space(new hnswlib::L2Space(100)),
      index(new hnswlib::HierarchicalNSW<float>(space, max_elements, M, ef_construction)),
      ef_search(ef_search) {}

// uild the HNSW index
void HnswDB::build_index(const vector<DataPoint>& lkher) {
    this->data = lkher;

    // Parallelize the addition of points to the index
    ParallelFor(0, this->data.size(), 0, [&](size_t i, size_t threadId) {
        index->addPoint(data[i].vector.data(), i);
    });

}

// Execute queries
vector<unsigned int> HnswDB::execute_query(const Query& query, unsigned int k, unsigned int expansionFactor) {
    vector<unsigned int> neighbors;
    vector<std::pair<float, hnswlib::labeltype>> results;
    index->setEf(ef_search);

    // Search for nearest neighbors
    if (query.query_type == 0) {
        results = index->searchKnnCloserFirst(query.vector.data(), k);
    } else {
        results = index->searchKnnCloserFirst(query.vector.data(), k*expansionFactor);
    }

    // Process results based on query type
    if (query.query_type == 0) {
        for (const auto& result : results) {
            neighbors.push_back(static_cast<unsigned int>(result.second));
        }
    } else {
        // Filter results based on constraints
        for (const auto& result : results) {
            unsigned int idx = result.second;
            const auto& point = data[idx];
            if (matches_constraints(query, point)) {
                neighbors.push_back(idx);
                if (neighbors.size() == k) break;
            }
        }
    }

    return neighbors;
}

// Check if a DataPoint matches query constraints
bool HnswDB::matches_constraints(const Query& query, const DataPoint& point) {
    if (query.query_type == 1 && query.category_value != point.category) return false;
    if (query.query_type == 2 && (point.timestamp < query.timestamp_start || point.timestamp > query.timestamp_end)) return false;
    if (query.query_type == 3 && (query.category_value != point.category ||
                                  point.timestamp < query.timestamp_start ||
                                  point.timestamp > query.timestamp_end)) return false;
    return true;
}
