#ifndef MPPAGERANK_GDSCACHE4_H
#define MPPAGERANK_GDSCACHE4_H

#include <unordered_map>

#include "../CacheItem.h"
#include "../Cache.h"

using namespace std; 

class GDSCache4: public Cache {

    private: 
        long double _L { 0 };
    public: 
        GDSCache4(long double m, long double t): Cache(m, t) { }
        
        bool refer(CacheItem *item); 
};

#endif