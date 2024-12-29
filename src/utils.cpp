#include <fstream>
#include <vector>
#include <unordered_set>
#include <cassert>
#include <iostream>
#include <array>
#include <functional>
#include "../include/utils.h"

using namespace std;

// Implementation of BPTreeDB
vector<DataPoint> read_binary(const string& file_path, int num_dimensions) {
    vector<DataPoint> data;
    ifstream ifs(file_path, ios::binary);
    if (!ifs) throw runtime_error("Cannot open file: " + file_path);

    unsigned int N;
    ifs.read(reinterpret_cast<char*>(&N), sizeof(unsigned int));

    data.resize(N);
    vector<float> buffer(num_dimensions);

    for (size_t i = 0; i < N; ++i) {
        ifs.read(reinterpret_cast<char*>(buffer.data()), num_dimensions * sizeof(float));

        data[i].category = buffer[0];
        data[i].timestamp = buffer[1];
        copy(buffer.begin() + 2, buffer.begin() + 102, data[i].vector.begin());
    }

    return data;
}


// Read queries from binary file
vector<Query> read_queries(const string& file_path) {
    ifstream ifs(file_path, ios::binary);
    if (!ifs) throw runtime_error("Cannot open query file: " + file_path);
    
    unsigned int num_queries;
    ifs.read(reinterpret_cast<char*>(&num_queries), sizeof(unsigned int));
    
    vector<Query> queries(num_queries);
    vector<float> buffer(104);  // 4 + 100 dimensions
    
    // Read all queries at once
    for (unsigned int i = 0; i < num_queries; ++i) {
        ifs.read(reinterpret_cast<char*>(buffer.data()), 104 * sizeof(float));
        
        Query& q = queries[i];
        q.query_type = static_cast<int>(buffer[0]);
        q.category_value = buffer[1];
        q.timestamp_start = buffer[2];
        q.timestamp_end = buffer[3];
        copy(buffer.begin() + 4, buffer.end(), q.vector.begin());
    }
    
    return queries;
}



void save_knn(const vector<vector<unsigned int>> &knns, const string &path) {
    ofstream ofs(path, ios::out | ios::binary);
    const int K = 100;
    const unsigned int N = knns.size();
    assert(knns.front().size() == K);
    for (unsigned i = 0; i < N; ++i) {
        assert(knns[i].size() == K);
        auto const &knn = knns[i];
        ofs.write(reinterpret_cast<char const *>(&knn[0]), K * sizeof(unsigned int));
    }
    ofs.close();
}



vector<vector<unsigned int>> RecallCalculator::load_ground_truth(const string &path, unsigned int K) {
    ifstream ifs(path, ios::in | ios::binary);
    if (!ifs) {
        throw runtime_error("Cannot open file: " + path);
    }

    // Read ground truth data
    vector<vector<unsigned int>> ground_truth;
    unsigned int index = 0;

    while (!ifs.eof()) {
        vector<unsigned int> knn(K);
        ifs.read(reinterpret_cast<char *>(knn.data()), K * sizeof(unsigned int));
        if (ifs.gcount() == 0) break; // End of file
        ground_truth.push_back(knn);
        index++;
    }

    ifs.close();
    return ground_truth;
}

double RecallCalculator::calculate_recall(vector<vector<unsigned int>> &hnsw_results,
                            vector<vector<unsigned int>> &ground_truth,
                                unsigned int K) {
    assert(hnsw_results.size() == ground_truth.size());
    
    int total_queries = hnsw_results.size();
    int total_correct = 0;

    // Calculate recall for each query
    for (int i = 0; i < total_queries; ++i) {

        unordered_set<unsigned int> ground_set(ground_truth[i].begin(), ground_truth[i].end());

        for (unsigned int &neighbor : hnsw_results[i]) {
            if (ground_set.find(neighbor) != ground_set.end()) {
                total_correct++;
            }
        }
    }

    // Compute overall recall
    return static_cast<double>(total_correct) / (total_queries * K);
}




vector<pair<int, int>> RecallCalculator::calculate_recall_by_type(
    const vector<pair<int, vector<unsigned int>>>& hnsw_results,
    const vector<vector<unsigned int>>& ground_truth,
    unsigned int K) {
    
    assert(hnsw_results.size() == ground_truth.size());
    
    // Initialize counters for each query type (0-3)
    vector<pair<int, int>> type_stats(4, {0, 0});  // {correct_matches, total_queries}
    
    // Process each query
    for (size_t i = 0; i < hnsw_results.size(); ++i) {
        const int query_type = hnsw_results[i].first;
        const auto& results = hnsw_results[i].second;
        assert(query_type >= 0 && query_type < 4);  // Ensure valid query type
        
        // Convert ground truth to set for O(1) lookups
        unordered_set<unsigned int> ground_set(ground_truth[i].begin(), ground_truth[i].end());
        
        // Count correct matches for this query
        int query_correct = 0;
        for (const unsigned int& neighbor : results) {
            if (ground_set.find(neighbor) != ground_set.end()) {
                query_correct++;
            }
        }
        
        // Update statistics for this query type
        type_stats[query_type].first += query_correct;   // Add correct matches
        type_stats[query_type].second += K;              // Add total possible matches
    }
    
    return type_stats;
}