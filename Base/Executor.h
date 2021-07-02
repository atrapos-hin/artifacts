#ifndef HRANK_EXECUTOR_H
#define HRANK_EXECUTOR_H

#include <unordered_map>
#include "Config.h"
#include "../TransitionMatrix/TransitionMatrix.h"
#include "../TransitionMatrix/ConstraintMatrix.h"

class Executor {
    Config* _config;

    unordered_map<string, int> _dimensions;
    unordered_map<string, TransitionMatrix*> _t_matrices;

public:
    Executor(Config* config);

    void batch_run();

    map<string, ConstraintMatrix *> buildConstraintMatrices(string metapath, vector<int> *dimensions);
    vector<TransitionMatrix*> buildTransitionMatrices(string metapath, vector<int> dimensions, map<string, ConstraintMatrix*> constraint_matrices, int* constraintIndex);
    vector<TransitionMatrix*> buildTransitionMatricesAdaptive(string metapath, vector<int> dimensions, vector<unsigned long long int> *non_zeros);
    void writeResults(TransitionMatrix* result, string metapath);

    void buildMNCSketches(vector<TransitionMatrix*> matrices);

    void freeConstraintMatrices(map<string, ConstraintMatrix *> constraint_matrices);
    void freeMatrices(algorithm_type algorithm, vector<TransitionMatrix*> matrices, TransitionMatrix *result, bool is_cached);
    void freeMatrices();

};
#endif //HRANK_EXECUTOR_H
