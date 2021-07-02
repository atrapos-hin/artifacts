#include <iostream>
#include <ctime>
#include <fstream>

#include "HRankSY.h"
#include "../MatrixMultiplier/DynamicOptimizer.h"
#include "../TransitionMatrix/ConstraintMatrix.h"
#include "../TransitionMatrix/TransitionMatrix.h"
#include "../MatrixMultiplier/MatrixMultiplier.h"
#include "../Utils.h"


using namespace std;
using namespace Eigen;


TransitionMatrix* HRankSY::run(string metapath, vector<TransitionMatrix*> &matrices, vector<int> dimensions) {

    Utils::debug_msg("Running HRank multiplication");

    TransitionMatrix *result = nullptr;

    if (this->_config->_algo == algorithm_type::Seq) {
        result = MatrixMultiplier::sequential(matrices, this->_config->_adaptive, this->_config->_max_memory, true);
    } else {
        long double cost = 0;
        result = MatrixMultiplier::dynamic(matrices, this->_config->_adaptive, this->_config->_max_memory, dimensions, this->_config->getDynOptimizerType(), true, cost);
    }

    return result;
}

HRankSY::HRankSY(Config *config) : _config(config) {}



