#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "Config.h"
#include "../Utils.h"

void Config::setQueryFile(const string &query_file) {
    _query_file = query_file;
}

const string &Config::getQueryFile() const {
    return _query_file;
}

void Config::setNodesDir(const string &nodes_dir) {
    _nodes_dir = nodes_dir;
}

void Config::setRelationsDir(const string &relations_dir) {
    _relations_dir = relations_dir;
}

void Config::printArgs() {
    std::cout
        << "Metapath: " << _metapath << endl
        << "Nodes dir: " << _nodes_dir << endl
        << "Relations dir: " << _relations_dir << endl << endl;
}

void Config::setAlgorithm(algorithm_type algo) {
    _algo = algo;
}

void Config::setOTreeExpansion(algorithm_type expansion) {
    _otree_expansion = expansion;

}

void Config::setAdaptive(bool adaptive) {
    _adaptive = adaptive;
}

void Config::setMaxMemory(int max_memory) {
    _max_memory = max_memory;
}

void Config::printOutputDir(string output_dir) {
    output_dir_ = output_dir;
}

const string &Config::getMetapath() const {
    return _metapath;
}

string Config::getAlgorithm() const {
    switch (Config::_algo) {
        case algorithm_type::Seq:   return "Seq";
        case algorithm_type::DynP:   return "DynP";
        case algorithm_type::OTreeA: return "OTree";
        case algorithm_type::Baseline1: return "Baseline1";
        case algorithm_type::Baseline2: return "Baseline2";
        default:      return "Unknown";
    }
}

void Config::setMetapath(const string &metapath) {
    _metapath = metapath;
}

algorithm_type Config::getOtreeExpansion() const {
    return _otree_expansion;
}

void Config::setDynOptimizerType(optimizer_type type) {
    this->_dyn_optimizer_type = type;
}

optimizer_type Config::getDynOptimizerType() const {
    return this->_dyn_optimizer_type;
}

void Config::setConstraint(tuple<string, string, string> constraint) {
    this->_constraint = constraint;
}

tuple<string, string, string> Config::getConstraint() const {
    return this->_constraint;
}

void Config::setL(int l) {
    this->_l = l;
}

void Config::setS(int s) {
    this->_s = s;
}

int Config::getL() const {
    return this->_l;
}

int Config::getS() const {
    return this->_s;
}

void Config::setCacheSize(float size) {
    this->_cache_size = size;
}

long double Config::getCacheSize() {
    return this->_cache_size;
}

long double Config::getCacheThreshold() {
    return this->_cache_size * this->_cache_threshold;
}

void Config::setCachePolicy(cache_type ct) {
    this->_cache_policy = ct;
}

cache_type Config::getCachePolicy() {
    return this->_cache_policy;
}