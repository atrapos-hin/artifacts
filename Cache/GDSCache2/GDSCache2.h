#ifndef MPPAGERANK_GDSCACHE2_H
#define MPPAGERANK_GDSCACHE2_H

#include <unordered_map>

#include "../CacheItem.h"
#include "../Cache.h"

using namespace std; 

class GDSCache2: public Cache {

    private: 
        long double _L { 0 };
    public: 
        GDSCache2(long double m, long double t): Cache(m, t) { }
        
        bool refer(CacheItem *item); 
};

#endif