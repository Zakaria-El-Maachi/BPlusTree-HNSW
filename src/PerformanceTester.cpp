#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <exception>
#include <unordered_map>
#include <chrono>
#include <numeric>
#include "../include/BPTreeDB.h"
#include "../include/BPTreeDB.h"
#include "../include/utils.h"

using namespace std;

class PerformanceTester {
private:
    VectorDatabase& db; // Reference to the chosen database
    vector<DataPoint>& data;
    unsigned int expansionFactor;
    double totalBuildingTime = 0;
    mutex queryMutex;
    vector<vector<unsigned int>> groundTruth;
    vector<vector<double>> queryTimes;
    vector<pair<int, int>> recalls;

public:
    PerformanceTester(VectorDatabase& database, const string& groundTruthPath, unsigned int K, unsigned int expansion, vector<DataPoint>& data0)
        : db(database), data(data0), expansionFactor(expansion) {
        groundTruth = RecallCalculator::load_ground_truth(groundTruthPath, K);
    }

    void build_index() {
        auto start = chrono::high_resolution_clock::now();
        db.build_index(data);
        auto end = chrono::high_resolution_clock::now();
        totalBuildingTime = chrono::duration<double>(end - start).count();
        cout << "Building Time: " << totalBuildingTime << endl;
    }

    void execute_queries(const vector<Query>& queries, unsigned int k, size_t numThreads = 0) {
        vector<pair<int, vector<unsigned int>>> results(queries.size());
        queryTimes.clear();
        queryTimes.resize(4);

        ParallelFor(0, queries.size(), numThreads, [&](size_t i, size_t threadId) {
            auto start = chrono::high_resolution_clock::now();
            auto result = db.execute_query(queries[i], k, expansionFactor);
            auto end = chrono::high_resolution_clock::now();

            double queryTime = chrono::duration<double>(end - start).count();
            {
                lock_guard<mutex> lock(queryMutex);
                results[i] = {queries[i].query_type, move(result)};
                queryTimes[queries[i].query_type].push_back(queryTime);
            }
        });

        recalls = RecallCalculator::calculate_recall_by_type(results, groundTruth, k);
    }

    void print_statistics() const {
        cout << "Performance Statistics:\n";

        cout << "Total Building Time: " << totalBuildingTime << "s\n";

        double globalTotalTime = 0.0;
        size_t globalQueryCount = 0;
        double globalRecallNumerator = 0.0; // Sum of correct results
        double globalRecallDenominator = 0.0; // Sum of all ground truth results

        for (int queryType = 0; queryType < 4; queryType++) {
            const vector<double>& times = queryTimes[queryType];
            double totalTime = accumulate(times.begin(), times.end(), 0.0);
            double avgTime = totalTime / times.size();
            double maxTime = *max_element(times.begin(), times.end());
            double minTime = *min_element(times.begin(), times.end());
            double qps = times.size() / totalTime;

            cout << "Query Type " << queryType << ":\n";
            cout << "  Avg Time = " << avgTime << "s\n";
            cout << "  Max Time = " << maxTime << "s\n";
            cout << "  Min Time = " << minTime << "s\n";
            cout << "  Queries Per Second (QPS) = " << qps << "\n";


            double recall = static_cast<double>(recalls[queryType].first) / recalls[queryType].second;
            cout << "  Recall = " << recall << "\n";


            // Aggregate for global statistics
            globalTotalTime += totalTime;
            globalQueryCount += times.size();
            globalRecallNumerator += recalls[queryType].first;
            globalRecallDenominator += recalls[queryType].second;
        }

        // Calculate global averages
        double globalAvgTime = globalQueryCount > 0 ? globalTotalTime / globalQueryCount : 0.0;
        double globalQPS = globalTotalTime > 0 ? globalQueryCount / globalTotalTime : 0.0;
        double globalRecall = globalRecallDenominator > 0 ? globalRecallNumerator / globalRecallDenominator : 0.0;

        cout << "\nGlobal Performance Statistics:\n";
        cout << "  Global Avg Time = " << globalAvgTime << "s\n";
        cout << "  Global Queries Per Second (QPS) = " << globalQPS << "\n";
        cout << "  Global Recall = " << globalRecall << "\n";
    }
};
