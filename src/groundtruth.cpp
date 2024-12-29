#include <thread>
#include <mutex>
#include <vector>
#include <future>
#include <algorithm>
#include <chrono>
#include <iostream>
#include "distances.cpp"
#include "../include/utils.h"
#include "../include/BPTreeDB.h"

using namespace std;
using namespace std::chrono;

int main() {
    try {
        // Initialize the VectorDatabase
        BPTreeDB db(euclideanDistance);

        // Specify file paths
        const string data_file = "data/contest-data-release-1m.bin";
        const string query_file = "data/contest-queries-release-1m.bin";
        const string knn_output_file = "output/output_knn.bin";

        // Read data and queries
        cout << "Reading data..." << endl;
        const vector<DataPoint> data = read_binary(data_file, 102);
        db.build_index(data);

        cout << "Reading queries..." << endl;
        vector<Query> queries = read_queries(query_file);

        const unsigned int k = 100;
        const unsigned int numThreads = thread::hardware_concurrency();

        cout << "Executing queries in parallel..." << endl;
        auto start = high_resolution_clock::now();

        vector<vector<unsigned int>> results(queries.size());

        // Use ParallelFor for parallel query execution
        ParallelFor(0, queries.size(), 0, [&](size_t i, size_t threadId) {
            results[i] = db.execute_query(queries[i], k);
        });

        auto end = high_resolution_clock::now();

        double total_time = duration<double>(end - start).count();
        cout << "Total time taken: " << total_time << " seconds" << endl;

        cout << "Saving KNN results to file: " << knn_output_file << endl;
        save_knn(results, knn_output_file); // Save KNN results to file (utils.cpp)

        cout << "Done!" << endl;
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << endl;
        return 1;
    }

    return 0;
}
