#include <iostream>
#include <cstring>
#include "./ext_libs/Eigen/Sparse"

#include "Utils.h"
#include "Base/Executor.h"
#include "Base/Config.h"

using namespace std;
using namespace Eigen;

const char *QUERY_FILE = "-qf";
const char *NODES_DIR_ARG = "-indir";
const char *RELATIONS_DIR_ARG = "-idir";
const char *ALGORITHM_ARG = "-algo";
const char *ADAPTIVE_ARG = "-ad";
const char *MAX_MEMORY_ARG = "-mem";
const char *OUTPUT_DIR_ARG = "-out";
const char *OTREE_EXPANSION = "-exp";
const char *DYNAMIC_OPTMIZER = "-dopt";
const char *OTREE_L = "-l";
const char *OTREE_S = "-s";
const char *CACHE_SIZE = "-cache";
const char *CACHE_POLICY = "-cachep";

int main(int argc, char* argv[]) {

    Config config;

    int i = 1;
    while (i < argc) {

        if (!strcmp(argv[i], QUERY_FILE)) {
            i = Utils::checkArg(i, argc);
            config.setQueryFile(argv[i]);

        } else if (!strcmp(argv[i], NODES_DIR_ARG)) {
            i = Utils::checkArg(i, argc);
            config.setNodesDir(argv[i]);

        } else if (!strcmp(argv[i], RELATIONS_DIR_ARG)) {
            i = Utils::checkArg(i, argc);
            config.setRelationsDir(argv[i]);

        } else if (!strcmp(argv[i], ALGORITHM_ARG)) {
            i = Utils::checkArg(i, argc);

            if (!strcmp(argv[i], "Seq")) {
                config.setAlgorithm(algorithm_type::Seq);
            } else if (!strcmp(argv[i], "DynP")) {
                config.setAlgorithm(algorithm_type::DynP);
            } else if (!strcmp(argv[i], "OTree")) {
                config.setAlgorithm(algorithm_type::OTreeA);
            } else if (!strcmp(argv[i], "Baseline1")) {
                config.setAlgorithm(algorithm_type::Baseline1);
            } else if (!strcmp(argv[i], "Baseline2")) {
                config.setAlgorithm(algorithm_type::Baseline2);
            } else {
                perror("Unrecognised Algorithm given");
                Utils::usage();
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(argv[i], ADAPTIVE_ARG)) {
            i = Utils::checkArg(i, argc);
            config.setAdaptive(!strcmp(argv[i], "1"));
        } else if (!strcmp(argv[i], MAX_MEMORY_ARG)) {
            i = Utils::checkArg(i, argc);
            config.setMaxMemory(atoi(argv[i]));
        } else if (!strcmp(argv[i], OUTPUT_DIR_ARG)) {
            i = Utils::checkArg(i, argc);
            config.printOutputDir(argv[i]);
        } else if (!strcmp(argv[i], OTREE_L)) {
            i = Utils::checkArg(i, argc);
            config.setL(atoi(argv[i]));
        } else if (!strcmp(argv[i], OTREE_S)) {
            i = Utils::checkArg(i, argc);
            config.setS(atoi(argv[i]));
        } else if (!strcmp(argv[i], OTREE_EXPANSION)) {
            i = Utils::checkArg(i, argc);
            if (!strcmp(argv[i], "Seq")) {
                config.setOTreeExpansion(algorithm_type::Seq);
            } else if (!strcmp(argv[i], "DynP")) {
                config.setOTreeExpansion(algorithm_type::DynP);
            } else if (!strcmp(argv[i], "Exp")) {
                config.setOTreeExpansion(algorithm_type::Exp);
            } else if (!strcmp(argv[i], "DynPB")) {
                config.setOTreeExpansion(algorithm_type::DynPB);
            } else if (!strcmp(argv[i], "ExpSparse")) {
                config.setOTreeExpansion(algorithm_type::ExpSparse);
            } else {
                perror("Unrecognised OTree expansion strategy given");
                Utils::usage();
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(argv[i], DYNAMIC_OPTMIZER)) {
            i = Utils::checkArg(i, argc);
            if (!strcmp(argv[i], "Sparse")) {
                config.setDynOptimizerType(optimizer_type::Sparse);
            } else if (!strcmp(argv[i], "Dense")) {
                config.setDynOptimizerType(optimizer_type::Dense);
            } else if (!strcmp(argv[i], "MNC")) {
                config.setDynOptimizerType(optimizer_type::MNC);
            } else {
                perror("Unrecognised Dynamic Optimizer Type given");
                Utils::usage();
                exit(EXIT_FAILURE);
            }
        } else if (!strcmp(argv[i], CACHE_SIZE)) {
            i = Utils::checkArg(i, argc);
            config.setCacheSize(atof(argv[i]));
        
        } else if (!strcmp(argv[i], CACHE_POLICY)) {
            i = Utils::checkArg(i, argc);
            if (!strcmp(argv[i], "LRU")) {
                config.setCachePolicy(cache_type::LRU);
            } else if (!strcmp(argv[i], "GDS")) {
                config.setCachePolicy(cache_type::GDS);
            } else if (!strcmp(argv[i], "GDS2")) {
                config.setCachePolicy(cache_type::GDS2);
            } else if (!strcmp(argv[i], "GDS3")) {
                config.setCachePolicy(cache_type::GDS3);
            } else if (!strcmp(argv[i], "GDS4")) {
                config.setCachePolicy(cache_type::GDS4);
            } else if (!strcmp(argv[i], "PGDSU")) {
                config.setCachePolicy(cache_type::PGDSU);
            } else {
                perror("Unrecognised Cache Policy given");
                Utils::usage();
                exit(EXIT_FAILURE);
            }
        } else {
            Utils::usage();
            exit(1);
        }
        i++;
    }

    #ifdef DEBUG_MSG
        config.printArgs();
    #endif

    Executor* exec = new Executor(&config);
    exec->batch_run();
    delete exec;

    return 0;
}