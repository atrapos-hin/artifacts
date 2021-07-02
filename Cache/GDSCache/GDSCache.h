#ifndef MPPAGERANK_GDSCACHE_H
#define MPPAGERANK_GDSCACHE_H

#include <unordered_map>

#include "../CacheItem.h"
#include "../Cache.h"

using namespace std; 

class GDSCache: public Cache {

    private: 
        long double _L { 0 };
    public: 
        GDSCache(long double m, long double t): Cache(m, t) {}
        
        bool refer(CacheItem *item); 
};

#endif