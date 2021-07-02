
#include "SimpleBaseline.h"
#include "../TransitionMatrix/TransitionMatrix.h"
#include "../MatrixMultiplier/MatrixMultiplier.h"

#include <iostream>
using namespace std;

TransitionMatrix* SimpleBaseline::run(string query, vector<TransitionMatrix*> &matrices, vector<int> dimensions, bool &resultIsCached) {
    resultIsCached = false;

    // return result if already in cache
    auto it = this->_result_map.find(query);
    if (it != this->_result_map.end() && it->second->isValid()) {
        this->_succReads++;
        resultIsCached = true;

        this->_cache->refer(it->second);
        return it->second->getMatrix();
    }

    this->_failedReads++;

    TransitionMatrix *result = nullptr;
    long double cost = 0;

    if (this->_config->getOtreeExpansion() == algorithm_type::Seq || this->_config->getOtreeExpansion() == algorithm_type::Exp)
        result = MatrixMultiplier::sequential(matrices, this->_config->_adaptive, this->_config->_max_memory, true);
    else if (this->_config->getOtreeExpansion() == algorithm_type::ExpSparse)
        result = MatrixMultiplier::expandSparse(matrices, this->_config->_adaptive, this->_config->_max_memory, true);
    else
        result = MatrixMultiplier::dynamic(matrices, this->_config->_adaptive, this->_config->_max_memory, dimensions, this->_config->getDynOptimizerType(), true, cost);

    auto *item = new CacheItem(this->_config->getConstraint(), result, cost, 0);

    if (item->getMemory() <= this->_cache->getItemThreshold()) {
        resultIsCached = true;

        // result was cached but is now invalid
        it = this->_result_map.find(query);
        if (it != this->_result_map.end()) {
            delete it->second;
            it->second = item;

        // cache result for first time 
        } else {
            this->_result_map.insert(pair<string, CacheItem*>(query, item));
        }
        this->_cache->refer(item);

        this->_writes++;
    }

    // this->_cache->display();

    return result;
}