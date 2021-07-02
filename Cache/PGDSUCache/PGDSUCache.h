#ifndef MPPAGERANK_PGDSU_CACHE_H
#define MPPAGERANK_PGDSU_CACHE_H

#include <unordered_map>

#include "../CacheItem.h"
#include "../Cache.h"
#include "../../ext_libs/GeneralizedSuffixTree/ST.h"

using namespace std; 
enum operation_type { INSERT, REMOVE };

class PGDSUCache: public Cache {

    private: 
        long double _L { 0 };
        ST* _stree;
    public: 
        PGDSUCache(long double m, long double t, ST* stree): Cache(m, t) { 
            this->_stree = stree;
        }
        
        bool refer(CacheItem *item); 
        void updateSubtreeScores(CacheItem * victim, operation_type op);
        void updateCacheOrder(CacheItem *item);
};


#endif