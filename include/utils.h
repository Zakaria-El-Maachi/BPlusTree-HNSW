#ifndef VECTOR_DATABASE_H
#define VECTOR_DATABASE_H

#include <fstream>
#include <vector>
#include <unordered_set>
#include <cassert>
#include <iostream>
#include <array>
#include <thread>
#include <atomic>
#include <mutex>
#include <exception>

using namespace std;

// Optimized data structure for vector points
struct DataPoint {
    float category;           // Discretized categorical attribute
    float timestamp;          // Normalized timestamp
    array<float, 100> vector; // Fixed-size array for better performance than vector
};

// Query structure
struct Query {
    int query_type;               // 0-3
    float category_value;         // v (-1 if not queried)
    float timestamp_start;        // l (-1 if not queried)
    float timestamp_end;          // r (-1 if not queried)
    array<float, 100> vector;     // query vector
};

// Abstract base class for vector database
class VectorDatabase {
public:
    virtual void build_index(const vector<DataPoint>& data) = 0;
    virtual vector<unsigned int> execute_query(const Query& query, unsigned int k, unsigned int expansionFactor) = 0;
    virtual ~VectorDatabase() = default;
};

// Utility functions
vector<DataPoint> read_binary(const string& file_path, int num_dimensions);
vector<Query> read_queries(const string& file_path);
void save_knn(const vector<vector<unsigned int>>& knns, const string& path = "output.bin");

// Recall calculator
class RecallCalculator {
public:
    static vector<vector<unsigned int>> load_ground_truth(const string& path, unsigned int K);
    static double calculate_recall(vector<vector<unsigned int>>& hnsw_results,
                                   vector<vector<unsigned int>>& ground_truth,
                                   unsigned int K);
    static vector<pair<int, int>> calculate_recall_by_type(
    const vector<pair<int, vector<unsigned int>>>& hnsw_results,
    const vector<vector<unsigned int>>& ground_truth,
    unsigned int K);
};

template <typename Function>
void ParallelFor(size_t start, size_t end, size_t numThreads, Function fn) {
    if (numThreads <= 0) {
        numThreads = std::thread::hardware_concurrency();
    }
    if (numThreads == 1) {
        for (size_t id = start; id < end; id++) {
            fn(id, 0);
        }
    } else {
        std::vector<std::thread> threads;
        std::atomic<size_t> current(start);

        // Handle exceptions from threads
        std::exception_ptr lastException = nullptr;
        std::mutex lastExceptMutex;

        for (size_t threadId = 0; threadId < numThreads; ++threadId) {
            threads.push_back(std::thread([&, threadId] {
                while (true) {
                    size_t id = current.fetch_add(1);
                    if (id >= end) break;

                    try {
                        fn(id, threadId);  // Call the function directly
                    } catch (...) {
                        std::unique_lock<std::mutex> lock(lastExceptMutex);
                        lastException = std::current_exception();
                        current = end;
                        break;
                    }
                }
            }));
        }

        for (auto& thread : threads) {
            thread.join();
        }

        if (lastException) {
            std::rethrow_exception(lastException);
        }
    }
}

#endif //VECTOR_DATABASE_H