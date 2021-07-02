
#include <iostream>
#include "GDSCache2.h"

/**
 * Indicates whether the first argument is considered to go before the second
 */
inline bool scoreCmp(const CacheItem* left, const CacheItem* right) {
    return left->getScore() < right->getScore(); 
}

// Refers key `item` within the LRU cache 
bool GDSCache2::refer(CacheItem *item) {

    long double itemMem = item->getMemory();         

    // item is larger that the pre-defined threshold
    if (itemMem > this->_itemThreshold) {
        return false;
    }

    // not in cache 
    if (this->_cacheRefs.find(item) == this->_cacheRefs.end()) { 

        // do not insert item that has less score than the min in cache
        if ((this->_curMemory + itemMem > this->_maxMemory) && !this->_cacheKeys.empty()) {
            CacheItem *victim = this->_cacheKeys.front(); 
            if (item->getScore() < victim->getScore()) {
                return false;
            }
        }
        
        // cache is full 
        while ((this->_curMemory + itemMem > this->_maxMemory) && !this->_cacheKeys.empty()) { 
        
            // delete least recently used element 
            CacheItem *victim = this->_cacheKeys.front(); 
            
            this->_L = victim->getScore();

            // Pops the first elmeent 
            this->_cacheKeys.pop_front(); 
  
            // Erase the first 
            this->_cacheRefs.erase(victim); 

            // free memory of deleted item
            this->_curMemory -= victim->getMemory();
            victim->deleteMatrix();
        }

        // add memory of current item to total   
        this->_curMemory += itemMem;

    // present in cache
    } else {
    
        this->_cacheKeys.erase(this->_cacheRefs[item]);

        // add L param only when an item is referenced
        // a newly inserted item is then referenced
        item->restoreScore(this->_L);
    }

    // insert item in sorted order based on item.score
    auto it = this->_cacheKeys.insert(
        upper_bound(this->_cacheKeys.begin(), this->_cacheKeys.end(), item, scoreCmp),
        item
    );

    // update reference 
    this->_cacheRefs[item] = it;

    return true;
}
