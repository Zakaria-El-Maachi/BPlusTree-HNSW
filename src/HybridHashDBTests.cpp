#include <iostream>
#include <vector>
#include <numeric>
#include <functional>
#include <thread>
#include <fstream>
#include "../include/utils.h"
#include "../include/HybridHashDB.h"
#include "../include/BPTreeDB.h"
#include "../include/HnswDB.h"
#include "../include/HashHnswDB.h"
#include "PerformanceTester.cpp"
#include "distances.cpp"

using namespace std;

int main() {
    ofstream output_file("output/hybrid_db_performance_output.txt");
    streambuf* cout_buf = cout.rdbuf();
    cout.rdbuf(output_file.rdbuf());

    const string ground_truth_path = "output/output_knn.bin";
    const unsigned int K = 100;

    // Read your data
    vector<DataPoint> data = read_binary("data/contest-data-release-1m.bin", 102); // Load your data
    vector<Query> queries = read_queries("data/contest-queries-release-1m.bin");



    // Create BPTreeDB, HnswDB, and HashHnswDB instances
    BPTreeDB bptree_db(euclideanDistance);
    HnswDB hnsw_db(16, 200, 500, 1000000);
    HashHnswDB hash_hnsw_db(16, 200, 500);

    cout << "\nRunning grid search on HybridDB..." << endl;

    HybridHashDB hybrid_db(bptree_db, hnsw_db, hash_hnsw_db, 0, 0);
    PerformanceTester tester(hybrid_db, ground_truth_path, K, 10, data);

    tester.build_index();

    // Define the search space for alpha and beta values
    vector<float> alpha_values = {0.5f, 0.4f, 0.3f};
    vector<float> beta_values = {0.4f, 0.3f, 0.2f};

    // Loop over all combinations of alpha and beta values
    for (float alpha : alpha_values) {
        for (float beta : beta_values) {
            cout << "\nTesting HybridDB with alpha=" << alpha << ", beta=" << beta << "..." << endl;

            hybrid_db.alpha = alpha;
            hybrid_db.beta = beta;

            tester.execute_queries(queries, K, 0);
            tester.print_statistics();
        }
    }

    cout.rdbuf(cout_buf);

    cout << "Done !" << endl;

    return 0;
}
