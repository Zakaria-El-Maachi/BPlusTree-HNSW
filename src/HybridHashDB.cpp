#include "../include/HybridHashDB.h"
#include <algorithm>
#include <stdexcept>

// Helper method to calculate query length
size_t HybridHashDB::calculate_query_length(const Query& query) const {
    auto lower = lower_bound(data.begin(), data.end(), query.timestamp_start, 
        [](const DataPoint& dp, float ts) { return dp.timestamp < ts; });

    auto upper = upper_bound(data.begin(), data.end(), query.timestamp_end, 
        [](float ts, const DataPoint& dp) { return ts < dp.timestamp; });

    return distance(lower, upper);
}

// Deciding which index to forward the query to
bool HybridHashDB::decide(const Query& query) {
    switch (query.query_type) {
        case 0: return true; // Forward to HnswDB
        case 1: return false; // Forward to HashHnswDB
        case 2: {
            size_t queryLength = calculate_query_length(query);
            return queryLength > static_cast<size_t>(alpha * data.size());
        }
        case 3: {
            size_t queryLength = calculate_query_length(query);
            return queryLength > static_cast<size_t>(beta * data.size());
        }
        default: throw invalid_argument("Unknown query type");
    }
}

// Constructor
HybridHashDB::HybridHashDB(BPTreeDB bpt, HnswDB hnsw, HashHnswDB hashHnsw, float alphaVal, float betaVal)
    : bpTreeDb(move(bpt)), hnswDb(move(hnsw)), hashHnswDb(move(hashHnsw)), alpha(alphaVal), beta(betaVal) {}

// Overridden method to build indexes for all the DB types
void HybridHashDB::build_index(const vector<DataPoint>& data0) {
    data = data0;

    bpTreeDb.build_index(data);
    hnswDb.build_index(data);
    hashHnswDb.build_index(data);

    // Sort data by timestamp for efficient query length computation
    sort(data.begin(), data.end(), [](const DataPoint& a, const DataPoint& b) {
        return a.timestamp < b.timestamp;
    });
}

// Overridden method to execute a query on the correct index
vector<unsigned int> HybridHashDB::execute_query(const Query& query, unsigned int k, unsigned int expansionFactor) {
    vector<unsigned int> results;

    if (query.query_type == 0) {
        results = hnswDb.execute_query(query, k, expansionFactor);
    } else if (query.query_type == 1) {
        results = hashHnswDb.execute_query(query, k, expansionFactor);
    } else {
        if (decide(query)) {
            if (query.query_type == 2) results = hnswDb.execute_query(query, k, expansionFactor);
            else hashHnswDb.execute_query(query, k, expansionFactor);
        } else {
            results = bpTreeDb.execute_query(query, k, expansionFactor);
        }
    }

    return results;
}
