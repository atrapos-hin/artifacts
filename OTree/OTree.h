#ifndef HRANK_OTREE_H
#define HRANK_OTREE_H

#include "../TransitionMatrix/TransitionMatrix.h"
#include "../ext_libs/GeneralizedSuffixTree/ST.h"
#include "../Base/Config.h"
#include "../Cache/Cache.h"

class OTree {
private:
    Config* _config;

    // min support of subpath
    unsigned int _s;

    // min length of subpath
    unsigned int _l;

    ST* _s_tree { nullptr };

    Cache *_cache { nullptr };

    unsigned int _succReads = 0;
    unsigned int _failedReads = 0;
    unsigned int _writes = 0;

    int _constraintIndex = -1;
public:
    OTree(Config *config);

    ~OTree();

    TransitionMatrix* run(string metapath, vector<TransitionMatrix*> &matrices, vector<int> dimensions, bool &is_cached_result, int constraintIndex);

    // TransitionMatrix* computeOTree(string metapath, vector<TransitionMatrix*> &matrices, vector<int> dimensions,
    //                         bool &is_cached_result, vector<NodeInfo*> missing_nodes, vector<NodeInfo*> cached_nodes);
    TransitionMatrix* computeOTreeDynamic(string metapath, vector<TransitionMatrix*> &matrices, vector<int> dimensions,
                            bool &is_cached_result, vector<NodeInfo*> missing_nodes, vector<NodeInfo*> cached_nodes);

    void findAllOccurences(std::vector<size_t> &vec, std::string data, NodeInfo* toSearch);
    NodeInfo* findSubpathToCache(vector<NodeInfo*> acc_nodes, NodeInfo* cached_node, string metapath, vector<TransitionMatrix*> &matrices, vector<int> dimensions);
    NodeInfo* findSubpathToUseFromCache(vector<NodeInfo*> acc_nodes);
    // NodeInfo* findSubpathToUseFromCache(vector<string> subpaths, vector<NodeInfo*> acc_nodes);
    NodeInfo* findSubpathToCache(/*vector<string> subpaths,*/ vector<NodeInfo*> acc_nodes, NodeInfo* cached_node);

    void fixSubpaths(vector<NodeInfo*> &nodes);
    TransitionMatrix* computeSubpath(string subpath, NodeInfo* cached_nodes, vector<TransitionMatrix *> matrices, size_t start, size_t len,
                                     vector<int> dimensions, long double &cost);
    TransitionMatrix* computeMetapathFromSubpath(string metapath, vector<TransitionMatrix*> matrices,
                                                        vector<int> dimensions, NodeInfo* cached_node, bool &is_cached_result, bool fitsInCache, long double &cost);
    NodeInfo* checkSubpathFromCache(string metapath, vector<NodeInfo*> acc_nodes);
    void separateSubpaths(vector<NodeInfo*> acc_nodes, vector<NodeInfo*> & missing_nodes, vector<NodeInfo*> & cached_nodes, unordered_map<string, NodeInfo*> & nodesWithSparsity);
    void delete_acc_nodes(vector<NodeInfo*> acc_nodes);
    void delete_matrices(vector<TransitionMatrix*> matrices);
    vector<NodeInfo*> filterNodesInPlan(vector<string> subpaths, vector<NodeInfo*> acc_nodes);
    
    unsigned int getCacheFailedReads() { return this->_failedReads; }
    unsigned int getCacheSuccReads() { return this->_succReads; }
    unsigned int getCacheWrites() { return this->_writes; }
};

#endif //HRANK_OTREE_H
