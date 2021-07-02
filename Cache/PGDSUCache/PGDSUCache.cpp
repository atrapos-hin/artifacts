
#include <iostream>
#include "PGDSUCache.h"

/**
 * Indicates whether the first argument is considered to go before the second
 */
inline bool scoreCmp(const CacheItem* left, const CacheItem* right) {
    return left->getScore2() < right->getScore2(); 
}

void PGDSUCache::updateCacheOrder(CacheItem *item) {

    // remove item from its current position
    this->_cacheKeys.erase(this->_cacheRefs[item]);

    // insert item in sorted order based on item.score
    auto it = this->_cacheKeys.insert(
        upper_bound(this->_cacheKeys.begin(), this->_cacheKeys.end(), item, scoreCmp),
        item
    );

    // update reference 
    this->_cacheRefs[item] = it;
}

void PGDSUCache::updateSubtreeScores(CacheItem *victim, operation_type op) {
    // cout << "OP: " << op << " CITEM: " << victim << " REL: " << victim->getMatrix()->get_relation() << " CON: " << get<2>(victim->getConstraint()) << endl;
    // cout << "CITEM NODE REF: -> " << victim->getNodeRef() << endl;

    vector<STnode*> subtreeNodes;

    // start from the 'victim' node and traverse its subtree
    this->_stree->traverseSubtree(victim->getNodeRef(), subtreeNodes);
    
    // check in subtree nodes for cached items with the same constratins
    for (auto &node : subtreeNodes) {

        CacheItem *cItem = node->getCachedResult(victim->getConstraint(), 0, false);
        if (cItem && cItem->isValid() && (victim != cItem)) {

            // there is a case where cItem is not yet in the cache at this point
            // as it is now inserted and the eviction of another cache item led here
            // however, cItem is on the tree so we can find it here
            if(this->_cacheRefs.find(cItem) == this->_cacheRefs.end()) {
                continue;
            }

            if (op == operation_type::REMOVE) {
                
                // cout << "\t-> add cost to: " << cItem->getMatrix()->get_relation() << "(" << get<2>(cItem->getConstraint()) << ")" << endl;
                cItem->addCost(victim->getCost(), this->_L);
                this->updateCacheOrder(cItem);
                
            } else if (op == operation_type::INSERT) {

                if (cItem->getCost() > victim->getCost()) {

                    // cout << "\t-> substract cost from: " << cItem->getMatrix()->get_relation() << "(" << get<2>(cItem->getConstraint()) << ")" << endl;
                    cItem->substractCost(victim->getCost(), this->_L);
                    this->updateCacheOrder(cItem);
                }

            }
        }
    }
}

// Refers key `item` within the LRU cache 
bool PGDSUCache::refer(CacheItem *item) {

    long double itemMem = item->getMemory();         

    // item is larger that the pre-defined threshold
    if (itemMem > this->_itemThreshold) {
        return false;
    }

    // not in cache 
    if (this->_cacheRefs.find(item) == this->_cacheRefs.end()) { 

        // cache is full 
        while ((this->_curMemory + itemMem > this->_maxMemory) && !this->_cacheKeys.empty()) { 
        
            // delete least recently used element 
            CacheItem *victim = this->_cacheKeys.front(); 

            this->_L = victim->getScore2();

            // Pops the first element
            this->_cacheKeys.pop_front(); 
  
            // Erase the first 
            this->_cacheRefs.erase(victim); 

            // free memory of deleted item
            this->_curMemory -= victim->getMemory();
            
            // cout << "\titem is evicted: " << victim->getMatrix()->get_relation() << "(" << get<2>(victim->getConstraint()) << ")" << endl;

            // update scores in subtree based on the item to be deleted
            this->updateSubtreeScores(victim, operation_type::REMOVE);
            
            // invalidate cache item and delete matrix
            victim->deleteMatrix();
        }

        // add memory of current item to total   
        this->_curMemory += itemMem;

    // present in cache
    } else {
    
        this->_cacheKeys.erase(this->_cacheRefs[item]);

        // add L param only when an item is referenced
        // a newly inserted item is then referenced
        item->restoreScore2(this->_L);
    }

    // insert item in sorted order based on item.score
    auto it = this->_cacheKeys.insert(
        upper_bound(this->_cacheKeys.begin(), this->_cacheKeys.end(), item, scoreCmp),
        item
    );

    // update reference 
    this->_cacheRefs[item] = it;

    // cout << "\titem is inserted: " << item->getMatrix()->get_relation() << "(" << get<2>(item->getConstraint()) << ")" << endl;

    // update scores in subtree based on the new inserted item
    this->updateSubtreeScores(item, operation_type::INSERT);

    return true;
}
