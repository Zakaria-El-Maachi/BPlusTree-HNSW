#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <exception>
#include <unordered_map>
#include <chrono>
#include <numeric>
#include <fstream>
#include "../include/BPTreeDB.h"
#include "../include/HnswDB.h"
#include "PerformanceTester.cpp"
#include "../include/utils.h"
#include "distances.cpp"

using namespace std;

int main() {
    ofstream output_file("output/performance_output.txt");
    streambuf* cout_buf = cout.rdbuf();
    cout.rdbuf(output_file.rdbuf());  

    const string ground_truth_path = "output/output_knn.bin";
    const unsigned int K = 100; 

    // Read your data
    vector<DataPoint> data = read_binary("data/contest-data-release-1m.bin", 102);
    vector<Query> queries = read_queries("data/contest-queries-release-1m.bin");

    // Test BPTreeDB once using PerformanceTester
    {
        cout << "Testing BPTreeDB..." << endl;
        BPTreeDB bptree_db(euclideanDistance); // Example parameters for BPTreeDB
        PerformanceTester tester_bptree(bptree_db, ground_truth_path, K, 1, data);

        tester_bptree.build_index();
        tester_bptree.execute_queries(queries, K, 0); // Example: 4 threads
        tester_bptree.print_statistics();
    }

    // Run grid search for HnswDB
    cout << "\nRunning grid search on HnswDB..." << endl;

    // Define the search space for HNSW parameters
    vector<int> M_values = {16, 32};
    vector<unsigned int> expansionFactor_values = {5, 10, 20};
    vector<int> ef_search_values = {200, 500, 1000};

    // Loop over all combinations of parameters in the grid
    for (int M : M_values) {
        for (int expansionFactor : expansionFactor_values) {
            for (int ef_search : ef_search_values) {
                cout << "\nTesting HnswDB with M=" << M << ", expansionFactor=" << expansionFactor << ", ef_search=" << ef_search << "..." << endl;

                // Create HnswDB with the current set of parameters
                HnswDB hnsw_db(M, 200, ef_search, 1000000);

                // Test with PerformanceTester
                PerformanceTester tester_hnsw(hnsw_db, ground_truth_path, K, expansionFactor, data);
                tester_hnsw.build_index();
                tester_hnsw.execute_queries(queries, K, 0);
                tester_hnsw.print_statistics();
            }
        }
    }

    cout.rdbuf(cout_buf);

    cout << "Done !" << endl;

    return 0;
}
