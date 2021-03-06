#ifndef HRANK_UTILS_H
#define HRANK_UTILS_H

#include <string>
#include <vector>
#include "./ext_libs/Eigen/Sparse"
#include "TransitionMatrix/TransitionMatrix.h"
#include "Cache/CacheItem.h"
// #include "./ext_libs/GeneralizedSuffixTree/STnode.h"

using namespace std;

class Utils {
public:
    static void usage();
    static int checkArg(int i, int max);
    static int get_column_idx(string filename, string fieldName);
    static int get_max_column_value(string filename, int column_idx);
    static void split(string line, vector<string> &tokens, char delim);
    static void print(Eigen::SparseMatrix<int> *matrix_);
    static void debug_msg(string msg);

    // TODO: merge two functions below (templates?)
    static vector<TransitionMatrix*> slice(vector<TransitionMatrix*> matrices, size_t start, size_t len);
    static vector<int> slice(vector<int> matrices, size_t start, size_t len);
    static void getMetapathAndConstraints(string query_line, string &metapath, tuple<string, string, string> &constraint);
    static void printConstraint(tuple<string, string, string> constraint);

    static long double getCurrentRSS();
    static size_t getPeakRSS();
};

enum algorithm_type { Seq, DynP, DynPB, OTreeA, Exp, ExpSparse, Baseline1, Baseline2 };
enum optimizer_type { Dense, Sparse, MNC };
enum cache_type { LRU, GDS, GDS2, GDS3, GDS4, PGDSU };

#endif //HRANK_UTILS_H
