#ifndef HRANK_CONFIG_H
#define HRANK_CONFIG_H

#include <string>
#include <vector>
#include <map>
#include "../Utils.h"

using namespace std;

class Config {
public:
    string _metapath;
    string _nodes_dir;
    string _relations_dir;
    string _query_file;
    algorithm_type _algo;
    bool _adaptive { false };
    int _max_memory { 0 };
    string output_dir_;
    algorithm_type _otree_expansion;
    optimizer_type _dyn_optimizer_type;
    tuple<string, string, string> _constraint;
    long double _cache_size;
    long double _cache_threshold = 0.8;
    cache_type _cache_policy;

    // otree parameters
    int _l = 3;
    int _s = 2;

    void setQueryFile(const string &query_file);

    void setNodesDir(const string &nodes_dir);

    void setRelationsDir(const string &relations_dir);

    void setConstraintsFile(const string &constraints_file);

    void setAlgorithm(algorithm_type algo);

    void setAdaptive(bool adaptive);

    void setMaxMemory(int max_memory);

    void setOTreeExpansion(algorithm_type expansion);

    void setDynOptimizerType(optimizer_type type);

    void setConstraint(tuple<string, string, string> constraint);

    void setL(int l);

    void setS(int s);

    void printArgs();

    void printOutputDir(string output_dir);

    void setMetapath(const string &metapath);

    const string &getMetapath() const;

    string getAlgorithm() const;

    const string &getQueryFile() const;

    algorithm_type getOtreeExpansion() const;

    optimizer_type getDynOptimizerType() const;

    tuple<string, string, string> getConstraint() const;

    int getL() const;
    int getS() const;

    void setCacheSize(float size);
    long double getCacheSize();
    long double getCacheThreshold();

    void setCachePolicy(cache_type ct);
    cache_type getCachePolicy();

};


#endif //HRANK_CONFIG_H
