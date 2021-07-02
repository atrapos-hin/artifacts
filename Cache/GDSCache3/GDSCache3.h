#ifndef MPPAGERANK_GDSCACHE3_H
#define MPPAGERANK_GDSCACHE3_H

#include <unordered_map>

#include "../CacheItem.h"
#include "../Cache.h"

using namespace std; 

class GDSCache3: public Cache {

    private: 
        long double _L { 0 };
    public: 
        GDSCache3(long double m, long double t): Cache(m, t) { }
        
        bool refer(CacheItem *item); 
};

#endif