#ifndef HRANK_MATRIXMULTIPLIER_H
#define HRANK_MATRIXMULTIPLIER_H


#include "../TransitionMatrix/TransitionMatrix.h"
#include "../Utils.h"
#include "../ext_libs/GeneralizedSuffixTree/ST.h"

class MatrixMultiplier {
public:
    static TransitionMatrix* sequential(vector<TransitionMatrix*> &matrices, bool adaptive, int max_memory, bool delete_input);
    static TransitionMatrix* dynamic(vector<TransitionMatrix*> &matrices, bool adaptive, int max_memory, vector<int> dimensions, optimizer_type sparse_optimizer, bool delete_input, long double &cost);
    static TransitionMatrix* expand(vector<TransitionMatrix*> &matrices, size_t subpath_pos, bool adaptive, int max_memory, bool delete_input);
    static TransitionMatrix* expandSparse(vector<TransitionMatrix*> &matrices, bool adaptive, int max_memory, bool delete_input);

    static vector<string> getSubpathsFromDynamicPlanning(vector<TransitionMatrix*> matrices, vector<int> dimensions);
    static vector<string> getSubpathsFromDynamicPlanningWithCache(vector<TransitionMatrix*> matrices, vector<int> dimensions, string metapath, unordered_map<string, NodeInfo*> cachedNodes, tuple<string, string, string> constraint, int _s, ST* stree);

    static long double estimateResultMemory(vector<int> dims, vector<TransitionMatrix*> matrices);
};


#endif //HRANK_MATRIXMULTIPLIER_H
