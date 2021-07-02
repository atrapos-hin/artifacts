#include <iostream>
#include "OTree.h"
#include "../Utils.h"
#include "../MatrixMultiplier/MatrixMultiplier.h"
#include "../Cache/LRUCache/LRUCache.h"
#include "../Cache/GDSCache/GDSCache.h"
#include "../Cache/GDSCache2/GDSCache2.h"
#include "../Cache/GDSCache3/GDSCache3.h"
#include "../Cache/GDSCache4/GDSCache4.h"
#include "../Cache/PGDSUCache/PGDSUCache.h"
#include <algorithm>
using namespace std;

TransitionMatrix* OTree::run(string metapath, vector<TransitionMatrix*> &matrices, vector<int> dimensions, bool &resultIsCached, int constraintIndex) {

    this->_constraintIndex = constraintIndex;

    #ifdef DEBUG_MSG
        // cout << "-------------------------------------" << endl;

        cout << "Metapath: " << metapath << endl;
        cout << endl;
    #endif

    metapath += '$';

    // returns nodes that have been accessed during insertion
    vector<NodeInfo*> acc_nodes = _s_tree->strInsertNaive(metapath, constraintIndex, this->_config->getConstraint());

    // delete '$' from the end of the metapath
    metapath = metapath.substr(0, metapath.size()-1);

    // update node counts
    _s_tree->updNodeCnts();

    // removes last '$' from subpaths and adjusts size
    // TODO: how can we avoid this ?
    this->fixSubpaths(acc_nodes);
    #ifdef DEBUG_MSG
        cout << "ACC_NODES: ";
        for (auto & c_node : acc_nodes) {
            cout << _s_tree->getRegSubStr(c_node->_str_id, c_node->_start, c_node->_size);
            cout << ", ";
        }
        cout << endl;
    #endif

    // get nodes that have cached results & those that need to be computed
    vector<NodeInfo*> missingNodes;
    vector<NodeInfo*> cachedNodes;
    unordered_map<string, NodeInfo*> nodesWithSparsity;
    this->separateSubpaths(acc_nodes, missingNodes, cachedNodes, nodesWithSparsity);
    
    // filter out nodes that represent subpaths that are not in the plan
    vector<string> computedSubpaths = MatrixMultiplier::getSubpathsFromDynamicPlanningWithCache(matrices, dimensions, metapath, nodesWithSparsity, this->_config->getConstraint(), this->_s, _s_tree); 
 	// vector<string> computedSubpaths = MatrixMultiplier::getSubpathsFromDynamicPlanning(matrices, dimensions);

    #ifdef DEBUG_MSG
        cout << "PLANNING: ";
        for (string s : computedSubpaths) {
            cout << s;
            cout << ", ";
        } 
        cout << endl;
    #endif
    
    missingNodes = this->filterNodesInPlan(computedSubpaths, missingNodes);
    cachedNodes = this->filterNodesInPlan(computedSubpaths, cachedNodes);
    #ifdef DEBUG_MSG

        cout << "MISSING: ";
        for (auto* node : missingNodes) {
            cout << _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size);
            cout << ", ";
        }
        cout << endl;

        cout << "CACHED: ";
        for (auto* node : cachedNodes) {
            cout << _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size);
            cout << ", ";
        }
        cout << endl;
    #endif

    TransitionMatrix* result = nullptr;

    // if (this->_config->getOtreeExpansion() == algorithm_type::Seq
    //     || this->_config->getOtreeExpansion() == algorithm_type::DynP
    //     || this->_config->getOtreeExpansion() == algorithm_type::Exp
    //     || this->_config->getOtreeExpansion() == algorithm_type::ExpSparse) {

    //     result = this->computeOTree(metapath, matrices, dimensions, resultIsCached, missingNodes, cachedNodes);

    // // use cached results according to dynamic programming plan
    // } else {
    result = this->computeOTreeDynamic(metapath, matrices, dimensions, resultIsCached, missingNodes, cachedNodes);
    // }

    // delete accessed nodes found during metapath insertion
    this->delete_acc_nodes(acc_nodes);
    
    // this->_cache->display();

    // print tree
    // _s_tree->print();

    return result;
}

vector<NodeInfo*> OTree::filterNodesInPlan(vector<string> subpaths, vector<NodeInfo*> acc_nodes) {
    vector<NodeInfo*> filtered;

    for (auto *node : acc_nodes) {
        string node_str = _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size);

        if(std::find(subpaths.begin(), subpaths.end(), node_str) != subpaths.end()) {
            filtered.push_back(node);
        }
    }
    return filtered;
}

TransitionMatrix* OTree::computeOTreeDynamic(string metapath, vector<TransitionMatrix*> &matrices, vector<int> dimensions,
                         bool &resultIsCached, vector<NodeInfo*> missingNodes, vector<NodeInfo*> cachedNodes) {

    if (missingNodes.empty() && cachedNodes.empty()) {
        long double cost = 0;
        return MatrixMultiplier::dynamic(matrices, this->_config->_adaptive, this->_config->_max_memory, dimensions, this->_config->getDynOptimizerType(), true, cost);
    }

    NodeInfo* cachedNode;
    vector<size_t> pos;
    string subpath;

    // vector<string> subpaths = MatrixMultiplier::getSubpathsFromDynamicPlanning(matrices, dimensions);

    cachedNode = findSubpathToUseFromCache(cachedNodes);
    if (cachedNode) {
        this->_succReads++;
    }

    NodeInfo* nodeToCache = nullptr;
    bool fitsInCache = true;

    // no cached record can be used
    // or a new missing subpath is larger that the current cached one
    if ((nodeToCache = findSubpathToCache(/*subpaths,*/ missingNodes, cachedNode)) != nullptr) {

        this->_failedReads++;

        auto constraint = (nodeToCache->_has_constraint) ? this->_config->getConstraint() : make_tuple("","","");

        #ifdef DEBUG_MSG
                cout << "\t3.1) Update cache needed" << endl;
        #endif

        // get string label for subpath to be cached
        subpath = _s_tree->getRegSubStr(nodeToCache->_str_id, nodeToCache->_start, nodeToCache->_size);

        #ifdef DEBUG_MSG
                cout << "\t3.2) Update cache for: " << subpath << " (Occ: " << nodeToCache->_node_ptr->getOccsNum(constraint) << ")" << endl;
        #endif

        this->findAllOccurences(pos, metapath, nodeToCache);
        if (pos.size() > 0) {
            // compute intermediate result to be cached
            long double subpathCost = 0;
            clock_t begin = clock();
            TransitionMatrix* subpathResult = this->computeSubpath(subpath, cachedNode, matrices, pos[0], subpath.size() - 1, dimensions, subpathCost);
            subpathCost = double(clock() - begin) / CLOCKS_PER_SEC;

            // create cache item
            int freq = nodeToCache->_node_ptr->getOccsNum(constraint);
            CacheItem *cacheItem = new CacheItem(constraint, subpathResult, subpathCost, freq);

            // cache intermediate result to the tree node
            // NOTE: it is always inserted in tree here as computeMetapathFromSubpath takes as argument a NodeInfo*
            // if it does not fit in cache, it is deleted from the tree before returning the final result
            nodeToCache->_node_ptr->setCachedResult(constraint, cacheItem);

            // set pointer from the cache item to the corresponding tree node
            cacheItem->setNodeRef(nodeToCache->_node_ptr);

            cachedNode = nodeToCache;
            
            // add current node to cached nodes
            cachedNodes.push_back(nodeToCache);

            // if it fits, insert item in cache
            fitsInCache = this->_cache->refer(cacheItem);
            if (fitsInCache) {
                this->_writes++;
            }
        }
    }

    // no cached result can be used and cache was not updated, so perform usual DynP multiplication
    if (cachedNode == nullptr) {
        long double cost = 0;
        return MatrixMultiplier::dynamic(matrices, this->_config->_adaptive, this->_config->_max_memory, dimensions, this->_config->getDynOptimizerType(), true, cost);
    }

    #ifdef DEBUG_MSG
        subpath = _s_tree->getRegSubStr(cachedNode->_str_id, cachedNode->_start, cachedNode->_size);
        cout << "\t3.3) Use subpath: " << subpath << endl;
    #endif

    // compute metapath based on cached result
    long double cost = 0;
    TransitionMatrix* result = this->computeMetapathFromSubpath(metapath, matrices, dimensions, cachedNode, resultIsCached, fitsInCache, cost);

    // if partial result does not fit in cache and is not exactly the returned result, then delete it from the tree
    if (!fitsInCache) {
        auto constraint = (cachedNode->_has_constraint) ? this->_config->getConstraint() : make_tuple("","","");
        auto *cacheItem = cachedNode->_node_ptr->getCachedResult(constraint, 0, true);
        cacheItem->setValid(false);

        if (!resultIsCached) {
            cacheItem->deleteMatrix();
        }
    }

    return result;
}

NodeInfo* OTree::findSubpathToCache(/*vector<string> subpaths,*/ vector<NodeInfo*> acc_nodes, NodeInfo* cached_node) {
    NodeInfo* node_ptr = nullptr;
    unsigned int tmp_size = 0;
    unsigned int tmp_occ_num = 0;
    int cached_node_len = (cached_node) ? cached_node->_size : 0;

    for (NodeInfo* node : acc_nodes) {
        auto constraint = (node->_has_constraint) ? this->_config->getConstraint() : make_tuple("","","");
        unsigned int occ_num = node->_node_ptr->getOccsNum(constraint);

        //cout << _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size) << " -> (" << node->_size << ")"  << " " << node->_node_ptr->getOccsNum() << " " << node->_node_ptr << endl;

        // string node_str = _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size);

        // if cached path is found in subpaths fron dynamic programming
        // if(std::find(subpaths.begin(), subpaths.end(), node_str) != subpaths.end()) {

            // choose subpath with greater count, larger when equals
            if (node->_size > cached_node_len
                && ((occ_num > tmp_occ_num) || ((occ_num == tmp_occ_num) && (node->_size > tmp_size)))) {

                tmp_size = node->_size;
                tmp_occ_num = occ_num;
                node_ptr = node;
            }
        // }
    }
    return node_ptr;
}

// NodeInfo* OTree::findSubpathToUseFromCache(vector<string> subpaths, vector<NodeInfo*> acc_nodes) {
//     NodeInfo* node_ptr = nullptr;
//     unsigned int tmp_size = 0;
//     unsigned int tmp_occ_num = 0;

//     for (auto &node : acc_nodes) {
//         auto constraint = (node->_has_constraint) ? this->_config->getConstraint() : make_tuple("","","");
//         unsigned int occ_num = node->_node_ptr->getOccsNum(constraint);
//         string node_str = _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size);


//         //cout << _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size) << " -> (" << node->_size << ")"  << " " << node->_node_ptr->getOccsNum() << " " << node->_node_ptr << endl;

//         // if cached path is found in subpaths fron dynamic programming
//         if(std::find(subpaths.begin(), subpaths.end(), node_str) != subpaths.end()) {

//             // choose larger subpath, with greater occ_num when equals
//             if ((node->_size > tmp_size) || ((node->_size == tmp_size) && (occ_num > tmp_occ_num))) {
//                 tmp_size = node->_size;
//                 tmp_occ_num = occ_num;
//                 node_ptr = node;
//             }
//         }
//     }
//     return node_ptr;
// }

// TransitionMatrix* OTree::computeOTree(string metapath, vector<TransitionMatrix*> &matrices, vector<int> dimensions,
//         bool &resultIsCached, vector<NodeInfo*> missingNodes, vector<NodeInfo*> cachedNodes) {

//     vector<size_t> pos;
//     string subpath;

//     NodeInfo* cachedNode;

//     // no new intermediate result to insert in cache
//     // no intermediate result to re-use from cache
//     if (missingNodes.empty() && cachedNodes.empty()) {

//         //cout << "1) No subpath to cache & no cached to use" << endl;
//         if (this->_config->getOtreeExpansion() == algorithm_type::Seq || this->_config->getOtreeExpansion() == algorithm_type::Exp)
//             return MatrixMultiplier::sequential(matrices, this->_config->_adaptive, this->_config->_max_memory, true);
//         else if (this->_config->getOtreeExpansion() == algorithm_type::ExpSparse)
//             return MatrixMultiplier::expandSparse(matrices, this->_config->_adaptive, this->_config->_max_memory, true);
//         else {
//             long double cost = 0;
//             return MatrixMultiplier::dynamic(matrices, this->_config->_adaptive, this->_config->_max_memory, dimensions, this->_config->getDynOptimizerType(), true, cost);
//         }
//     }

//     // choose which cached result to choose
//     cachedNode = findSubpathToUseFromCache(cachedNodes);
//     if (cachedNode) {
//         this->_succReads++;
//     }

//     NodeInfo* nodeToCache = nullptr;
//     bool fitsInCache = true;

//     // no cached record can be used
//     // or a new missing subpath is larger that the current cached one
//     if ((nodeToCache = findSubpathToCache(missingNodes, cachedNode, metapath, matrices, dimensions)) != nullptr) {

//         this->_failedReads++;

//         #ifdef DEBUG_MSG
//                 cout << "\t3.1) Update cache needed" << endl;
//         #endif

//         // get string label for subpath to be cached
//         subpath = _s_tree->getRegSubStr(nodeToCache->_str_id, nodeToCache->_start, nodeToCache->_size);
//         auto constraint = (nodeToCache->_has_constraint) ? this->_config->getConstraint() : make_tuple("","","");

//         #ifdef DEBUG_MSG
//                 cout << "\t3.2) Update cache for: " << subpath << " (Occ: " << nodeToCache->_node_ptr->getOccsNum(constraint) << ")" << endl;
//         #endif

//         this->findAllOccurences(pos, metapath, nodeToCache);

//         // compute intermediate result to be cached
//         long double subpathCost = 0;
//         TransitionMatrix* subpathResult = this->computeSubpath(subpath, cachedNodes, matrices, pos[0], subpath.size() - 1, dimensions, subpathCost);

//         // create cache item
//         int freq = nodeToCache->_node_ptr->getOccsNum(constraint);
//         CacheItem *cacheItem = new CacheItem(constraint, subpathResult, subpathCost, freq);

//         // cache intermediate result to the tree node
//         // NOTE: it is always inserted in tree here as computeMetapathFromSubpath takes as argument a NodeInfo*
//         // if it does not fit in cache, it is deleted from the tree before returning the final result
//         nodeToCache->_node_ptr->setCachedResult(constraint, cacheItem);
        
//         // set pointer from the cache item to the corresponding tree node
//         cacheItem->setNodeRef(nodeToCache->_node_ptr);
//         cachedNode = nodeToCache;
        
//         // add current node to cached nodes
//         cachedNodes.push_back(nodeToCache);

//         // if it fits, insert item in cache
//         fitsInCache = this->_cache->refer(cacheItem);
//         if (fitsInCache) {
//             this->_writes++;
//         }
//     }

//     // if we don't have a cached node and no other frequent node was able to be cached
//     // then proceed with normal multiplication without caching
//     if (!cachedNode) {
//         //cout << "1) No subpath to cache & no cached to use" << endl;
//         if (this->_config->getOtreeExpansion() == algorithm_type::Seq || this->_config->getOtreeExpansion() == algorithm_type::Exp)
//             return MatrixMultiplier::sequential(matrices, this->_config->_adaptive, this->_config->_max_memory, true);
//         else if (this->_config->getOtreeExpansion() == algorithm_type::ExpSparse)
//             return MatrixMultiplier::expandSparse(matrices, this->_config->_adaptive, this->_config->_max_memory, true);
//         else {
//             long double cost = 0;
//             return MatrixMultiplier::dynamic(matrices, this->_config->_adaptive, this->_config->_max_memory, dimensions, this->_config->getDynOptimizerType(), true, cost);
//         } 
//     }

//     #ifdef DEBUG_MSG
//         subpath = _s_tree->getRegSubStr(cachedNode->_str_id, cachedNode->_start, cachedNode->_size);
//         cout << "\t3.3) Use subpath: " << subpath << endl;
//     #endif

//     // compute metapath based on cached result
//     long double cost = 0;
//     TransitionMatrix* result = this->computeMetapathFromSubpath(metapath, matrices, dimensions, cachedNode, resultIsCached, fitsInCache, cost);

//     // if partial result does not fit in cache and is not exactly the returned result, then delete it from the tree
//     if (!fitsInCache) {
//         auto constraint = (cachedNode->_has_constraint) ? this->_config->getConstraint() : make_tuple("","","");
//         auto *cacheItem = cachedNode->_node_ptr->getCachedResult(constraint, 0, true);
//         cacheItem->setValid(false);
        
//         if (!resultIsCached) {
//             cacheItem->deleteMatrix();
//         }
//     }

//     return result;
// }

TransitionMatrix* OTree::computeMetapathFromSubpath(string metapath, vector<TransitionMatrix*> matrices,
        vector<int> dimensions, NodeInfo* cachedNode, bool &resultIsCached, bool fitsInCache, long double &cost) {

    string cachedSubpath = _s_tree->getRegSubStr(cachedNode->_str_id, cachedNode->_start, cachedNode->_size);

    #ifdef DEBUG_MSG
        cout << "\t4.1) Use cached subpath: " << cachedSubpath << endl;
    #endif

    vector<size_t> cachedStartIndices;
    this->findAllOccurences(cachedStartIndices, metapath, cachedNode);

    vector<TransitionMatrix*> metapathMatrices;
    vector<int> metapathDimensions;

    // dimension of first relation is fixed
    metapathDimensions.push_back(dimensions[0]);

    size_t c_pos = 0;       // index in the cached subpath string
    size_t m_pos = 0;       // index in the metapath string
    size_t subpath_pos = 0;

    while(m_pos < metapath.size()-1) {
        //cout << m_pos << ": " << metapath[m_pos] << endl;

        // if a cached result can be used from this position
        if (c_pos < cachedStartIndices.size() 
                    && cachedStartIndices[c_pos] == m_pos) {
            
            //cout << "Found at position: " << cached_start_indices[c_pos] << endl;
            auto constraint = (cachedNode->_has_constraint) ? this->_config->getConstraint() : make_tuple("","","");
            CacheItem *cachedItem = cachedNode->_node_ptr->getCachedResult(constraint, this->_s, true);
            TransitionMatrix* m = cachedItem->getMatrix();
            if (fitsInCache) {
                this->_cache->refer(cachedItem);
            }

            metapathMatrices.push_back(m);
            subpath_pos = metapathMatrices.size() - 1;

            c_pos++;
            m_pos += cachedSubpath.size() - 1;
            //cout << "Go to index: " << m_pos << endl;
            metapathDimensions.push_back(dimensions[m_pos]);

        } else {
            metapathMatrices.push_back(matrices[m_pos]);

            m_pos++;
            metapathDimensions.push_back(dimensions[m_pos]);
        }
    }

    // result found in cache
    if (metapathMatrices.size() == 1) {
        resultIsCached = true;
        return metapathMatrices[0];
    }

    if (this->_config->getOtreeExpansion() == algorithm_type::Seq)
        return MatrixMultiplier::sequential(metapathMatrices, this->_config->_adaptive, this->_config->_max_memory, false);
    else if (this->_config->getOtreeExpansion() == algorithm_type::DynP || this->_config->getOtreeExpansion() == algorithm_type::DynPB)
        return MatrixMultiplier::dynamic(metapathMatrices, this->_config->_adaptive, this->_config->_max_memory, metapathDimensions, this->_config->getDynOptimizerType(), false, cost);
    else if (this->_config->getOtreeExpansion() == algorithm_type::ExpSparse)
        return MatrixMultiplier::expandSparse(metapathMatrices, this->_config->_adaptive, this->_config->_max_memory, false);
    else    // Expand OTree Expansion Type
        return MatrixMultiplier::expand(metapathMatrices, subpath_pos, this->_config->_adaptive, this->_config->_max_memory, false);
}

void OTree::separateSubpaths(vector<NodeInfo*> acc_nodes, vector<NodeInfo*> & missing_nodes, vector<NodeInfo*> & cachedNodes, unordered_map<string, NodeInfo*> & nodesWithSparsity) {
    for (auto &node : acc_nodes) {
        // cout << this->_s_tree->getRegSubStr(node->_str_id, node->_start, node->_size) << "\t";

        auto constraint = (node->_has_constraint) ? this->_config->getConstraint() : make_tuple("","","");

        // ignore node if shorter that _l or less frequent that _s
        if (node->_size < this->_l || node->_node_ptr->getOccsNum(constraint) < this->_s)
            continue;

        CacheItem *cached_item = node->_node_ptr->getCachedResult(constraint, this->_s, true);
        
        if (cached_item == nullptr) {
        
            missing_nodes.push_back(node);
        
        } else {
            string node_str = this->_s_tree->getRegSubStr(node->_str_id, node->_start, node->_size);
            nodesWithSparsity[node_str] = node;

            if (cached_item->isValid()) {
                cachedNodes.push_back(node);
            }            
        }
    }
}

TransitionMatrix* OTree::computeSubpath(string subpath, NodeInfo* node_to_use, vector<TransitionMatrix *> matrices,
                     size_t start, size_t len, vector<int> dimensions, long double &cost) {

    //cout << "CACHED NODES:" << endl;
    //
    //for (auto & c_node : cachedNodes) {
    //    cout <<_s_tree->getRegSubStr(c_node->_str_id, c_node->_start, c_node->_size) << endl;
    //}

    vector<TransitionMatrix*> subpath_matrices = Utils::slice(matrices, start, len);
    vector<int> subpath_dimensions = Utils::slice(dimensions, start, len + 1);

    // NodeInfo* node_to_use = this->checkSubpathFromCache(subpath, cachedNodes);
    if (node_to_use != nullptr) {
        bool is_cached;
        return this->computeMetapathFromSubpath(subpath, subpath_matrices, subpath_dimensions, node_to_use, is_cached, true, cost);
    } else {
        #ifdef DEBUG_MSG
                cout << "\t3.3) No cached subpath can be used" << endl;
        #endif

        if (this->_config->getOtreeExpansion() == algorithm_type::Seq || this->_config->getOtreeExpansion() == algorithm_type::Exp)
            return MatrixMultiplier::sequential(subpath_matrices, this->_config->_adaptive, this->_config->_max_memory, false);
        else if (this->_config->getOtreeExpansion() == algorithm_type::ExpSparse)
            return MatrixMultiplier::expandSparse(subpath_matrices, this->_config->_adaptive, this->_config->_max_memory, false);
        else
            return MatrixMultiplier::dynamic(subpath_matrices, this->_config->_adaptive, this->_config->_max_memory, subpath_dimensions, this->_config->getDynOptimizerType(), false, cost);
    }
}

void OTree::fixSubpaths(vector<NodeInfo*> &nodes) {
    for (auto &node : nodes) {
        string subpath_str = _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size);

        // get last character of subpath
        string last_char = _s_tree->getRegSubStr(node->_str_id, node->_start + node->_size - 1, 1);

        if (last_char == "$") {
            node->_size--;
        }
    }
}

NodeInfo* OTree::findSubpathToUseFromCache(vector<NodeInfo*> acc_nodes) {
    NodeInfo* node_ptr = nullptr;
    unsigned int tmp_size = 0;
    unsigned int tmp_occ_num = this->_s;

    for (auto &node : acc_nodes) {
        auto constraint = (node->_has_constraint) ? this->_config->getConstraint() : make_tuple("","","");

        unsigned int occ_num = node->_node_ptr->getOccsNum(constraint);

        //cout << _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size) << " -> (" << node->_size << ")"  << " " << node->_node_ptr->getOccsNum() << " " << node->_node_ptr << endl;

        // choose larger subpath, with greater occ_num when equals
        if ( (node->_size > tmp_size) || ((node->_size == tmp_size) && (occ_num > tmp_occ_num)) ) {
            tmp_size = node->_size;
            tmp_occ_num = occ_num;
            node_ptr = node;
        }
    }
    return node_ptr;
}

//TODΟ: merge this with function above
NodeInfo* OTree::checkSubpathFromCache(string metapath, vector<NodeInfo*> acc_nodes) {

    NodeInfo* node_ptr = nullptr;
    unsigned int tmp_size = 0;
    unsigned int tmp_occ_num = this->_s;

    for (auto &node : acc_nodes) {

        unsigned int occ_num = node->_node_ptr->getOccsNum(this->_config->getConstraint());

        //cout << _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size) << " -> (" << node->_size << ")"  << " " << node->_node_ptr->getOccsNum() << " " << node->_node_ptr << endl;
        string node_path = _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size);
        size_t pos = metapath.find(node_path);

        // sub-path not found
        if (pos == string::npos) {
            continue;
        }

        // choose larger subpath, with greater occ_num when equals
        if ( (node->_size > tmp_size) || ((node->_size == tmp_size) && (occ_num > tmp_occ_num)) ) {
            tmp_size = node->_size;
            tmp_occ_num = occ_num;
            node_ptr = node;
        }
    }
    return node_ptr;
}

NodeInfo* OTree::findSubpathToCache(vector<NodeInfo*> missing_nodes, NodeInfo* cached_node, string metapath, vector<TransitionMatrix*> &matrices, vector<int> dimensions) {
    NodeInfo* node_ptr = nullptr;
    unsigned int tmp_size = 0;
    unsigned int tmp_occ_num = this->_s;
    int cached_node_len = (cached_node) ? cached_node->_size : 0;

    for (auto &node : missing_nodes) {
        
        auto constraint = (node->_has_constraint) ? this->_config->getConstraint() : make_tuple("","","");

        unsigned int occ_num = node->_node_ptr->getOccsNum(constraint);

        //cout << "\tfindSubpathToCache: " <<_s_tree->getRegSubStr(node->_str_id, node->_start, node->_size) << " -> (" << node->_size << ")"  << " " << node->_node_ptr->getOccsNum(this->_config->getConstraint()) << " " << node->_node_ptr << endl;

        // choose subpath with greater count, larger when equals
        if ( node->_size > cached_node_len
               && ((occ_num > tmp_occ_num) || ((occ_num == tmp_occ_num) && (node->_size > tmp_size)))) {

            vector<size_t> pos;
            this->findAllOccurences(pos, metapath, node);

            string subpath = _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size);
            size_t start = pos[0];
            size_t len = subpath.size() - 1;
            vector<TransitionMatrix*> subpath_matrices = Utils::slice(matrices, start, len);
            vector<int> subpath_dimensions = Utils::slice(dimensions, start, len + 1);
            
            // cout << "SUBPATH: " << subpath << endl;
            // cout << "SUBPATH EST MEM: " << MatrixMultiplier::estimateResultMemory(subpath_dimensions, subpath_matrices) << endl;

            if (MatrixMultiplier::estimateResultMemory(subpath_dimensions, subpath_matrices) < 5000) {
                tmp_size = node->_size;
                tmp_occ_num = occ_num;
                node_ptr = node;
            }
        }
    }

    return node_ptr;
}

void OTree::findAllOccurences(std::vector<size_t> &vec, std::string data, NodeInfo* node)
{
    string needle = _s_tree->getRegSubStr(node->_str_id, node->_start, node->_size);

    // Get the first occurrence
    size_t pos = data.find(needle);

    // Repeat till end is reached
    while(pos != std::string::npos)
    {
        // Add position to the vector

        if (node->_has_constraint) {

            // if we have constraint, check for position for subpath with the constrained included
            if ( pos <= this->_constraintIndex && (pos + needle.size()-1 >= this->_constraintIndex) ) {
                vec.push_back(pos);
                // cout << "POS: " << pos << ", END: " << pos + needle.size()-1 << " AND CINDEX: " << this->_constraintIndex << endl;
            }
        } else {
            vec.push_back(pos);
            // cout << " EDW POS: " << pos << ", END: " << pos + needle.size()-1 << " AND CINDEX: " << this->_constraintIndex << endl;
        }
        

        // Get the next occurrence from the current position
        pos = data.find(needle, pos + needle.size()-1);
    }
}

OTree::OTree(Config *config) : _config(config) {
    _s_tree = new ST();
    _l = config->getL();
    _s = config->getS();

    if (this->_config->getCachePolicy() == cache_type::GDS) {
        _cache = new GDSCache(config->getCacheSize(), config->getCacheThreshold());
    } else if (this->_config->getCachePolicy() == cache_type::GDS2) {
        _cache = new GDSCache2(config->getCacheSize(), config->getCacheThreshold());
    } else if (this->_config->getCachePolicy() == cache_type::GDS3) {
        _cache = new GDSCache3(config->getCacheSize(), config->getCacheThreshold());
    } else if (this->_config->getCachePolicy() == cache_type::GDS4) {
        _cache = new GDSCache4(config->getCacheSize(), config->getCacheThreshold());
    } else if (this->_config->getCachePolicy() == cache_type::LRU) {
        _cache = new LRUCache(config->getCacheSize(), config->getCacheThreshold());
    } else if (this->_config->getCachePolicy() == cache_type::PGDSU) {
        _cache = new PGDSUCache(config->getCacheSize(), config->getCacheThreshold(), this->_s_tree);
    }  
}

OTree::~OTree() {
    delete _s_tree;
    delete _cache;
}

void OTree::delete_acc_nodes(vector<NodeInfo*> acc_nodes) {
    for (auto & node : acc_nodes) {
        delete node;
    }
}

void OTree::delete_matrices(vector<TransitionMatrix*> matrices) {
    for (auto & m : matrices) {
        delete m;
    }
}


