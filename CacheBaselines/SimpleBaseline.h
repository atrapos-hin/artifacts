#ifndef SIMPLE_BASELINE_H
#define SIMPLE_BASELINE_H

#include "../Base/Config.h"
#include "../Cache/LRUCache/LRUCache.h"
#include "../TransitionMatrix/TransitionMatrix.h"

#include <unordered_map> 

class SimpleBaseline {
private:
    Config* _config;
    LRUCache *_cache { nullptr };
    unordered_map<string, CacheItem*> _result_map;   

    unsigned int _succReads = 0;
    unsigned int _failedReads = 0;
    unsigned int _writes = 0;

public:

    SimpleBaseline(Config *config) {
        this->_config = config;
        this->_cache = new LRUCache(config->getCacheSize(), config->getCacheThreshold());
    };

    ~SimpleBaseline() {
        delete this->_cache;
    }

    TransitionMatrix* run(string metapath, vector<TransitionMatrix*> &matrices, vector<int> dimensions, bool &resultIsCached);

    unsigned int getCacheFailedReads() { return this->_failedReads; }
    unsigned int getCacheSuccReads() { return this->_succReads; }
    unsigned int getCacheWrites() { return this->_writes; }
};


#endif //SIMPLE_BASELINE_H
