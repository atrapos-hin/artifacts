#include <fstream>
#include "Executor.h"
#include "../TransitionMatrix/TransitionMatrix.h"
#include "../HRank/HRankSY.h"
#include "../OTree/OTree.h"
#include "../Utils.h"
#include "../CacheBaselines/SimpleBaseline.h"
#include "../CacheBaselines/AdvancedBaseline.h"
#include <numeric>
using namespace Eigen;
using namespace std;
Executor::Executor(Config* config) : _config(config) {}

map<string, ConstraintMatrix*> Executor::buildConstraintMatrices(string metapath, vector<int> *dimensions) {
    map<string, ConstraintMatrix*> constraint_matrices;

    string constraint_node = get<0>(this->_config->getConstraint());

    // get max id values && read constraint matrices
    for(unsigned int i=0; i<metapath.size(); i++) {

        // convert char to string
        string node_name(1, metapath[i]);

        // find dimension of the constraint table
        string node_data_file = this->_config->_nodes_dir + node_name + ".csv";

        int dim = -1;
        auto dim_record = this->_dimensions.find(node_name);
        if (dim_record == this->_dimensions.end()) {
            int id_idx = Utils::get_column_idx(node_data_file, "id");
            dim = Utils::get_max_column_value(node_data_file, id_idx) + 1;
            this->_dimensions.insert(make_pair(node_name, dim));
        } else {
            dim = dim_record->second;
        }

        dimensions->push_back(dim);

        // if constraints are given for this node type and constraint matrix is not already created
        // then create constraint matrix
        if ( (constraint_node == node_name) && (constraint_matrices.find(node_name) == constraint_matrices.end()) ) {
            auto *c_map = new map<string, string>();
            c_map->emplace(get<1>(this->_config->getConstraint()), get<2>(this->_config->getConstraint()));

            auto *matrix = new ConstraintMatrix(node_name, node_data_file, dim);
            matrix->build(c_map);
            delete c_map;

            //matrix.print();
            constraint_matrices.emplace(node_name, matrix);
        }
    }

    return constraint_matrices;
}

void Executor::freeConstraintMatrices(map<string, ConstraintMatrix *> constraint_matrices) {

    auto it = constraint_matrices.begin();

    while (it != constraint_matrices.end()) {
        delete it->second;
        it++;
    }
}

vector<TransitionMatrix*> Executor::buildTransitionMatrices(string metapath, vector<int> dimensions, map<string, ConstraintMatrix*> constraint_matrices, int* constraintIndex) {

    vector<TransitionMatrix*> matrices;

    // used to apply constraints only on first node found
    // eg APCPA - we need to apply constraints of a
    bool constraints_applied = false;

    for(unsigned int i=0; i<metapath.size()-1; i++) {
        string src(1, metapath[i]);
        string trg(1, metapath[i+1]);

        string rel = src + trg;

        TransitionMatrix* transition_matrix = new TransitionMatrix();

        auto m_record = this->_t_matrices.find(rel);

        // matrix should be loaded from file
        if (m_record == this->_t_matrices.end()) {
            TransitionMatrix *tm = new TransitionMatrix(rel, this->_config->_relations_dir, dimensions[i], dimensions[i+1]);
            tm->read();

            this->_t_matrices.insert(make_pair(rel, tm));
            transition_matrix->copy(*tm);

        // matrix is already loaded
        } else {
            transition_matrix->copy(*(m_record->second));
        }

        if (!constraints_applied) {
            SparseMatrix<int> *t = transition_matrix->get_matrix();

            // apply constraints of source node type
            auto it = constraint_matrices.find(src);
            if (it != constraint_matrices.end()) {
                *t = *(it->second->get_matrix()) * (*t);
                constraints_applied = true;
                *constraintIndex = i;

            }

            // apply constraints of target node type
            it = constraint_matrices.find(trg);
            if (it != constraint_matrices.end()) {
                *t = (*t) * *(it->second->get_matrix());
                constraints_applied = true;
                *constraintIndex = i;

            }

        }
        // cout << rel << " = " << transition_matrix->get_matrix()->nonZeros() << endl;

        matrices.push_back(transition_matrix);
    }

    return matrices;
}

vector<TransitionMatrix*> Executor::buildTransitionMatricesAdaptive(string metapath, vector<int> dimensions,
                                                                      vector<unsigned long long int> *non_zeros) {

    vector<TransitionMatrix*> matrices;

    for(unsigned int i=0; i<metapath.size()-1; i++) {
        string src(1, metapath[i]);
        string trg(1, metapath[i + 1]);

        string rel = src + trg;

        //cout << "\t" << rel;
        TransitionMatrix *transition_matrix = new TransitionMatrix(rel, this->_config->_relations_dir, dimensions[i], dimensions[i + 1]);

        non_zeros->push_back(transition_matrix->non_zeros());
        matrices.push_back(transition_matrix);
    }
    return matrices;
}

void Executor::writeResults(TransitionMatrix* result, string metapath) {

    if (result->is_in_memory()) {

        if (this->_config->output_dir_.empty()) {

            // print to stdout
            Utils::print(result->get_matrix());
        } else {
            // or write results on disk
            string output_file = this->_config->output_dir_ + "/" + metapath + "_" + this->_config->getAlgorithm() + ".csv";
            ofstream fd = ofstream(output_file);
            result->write_to_file(fd, result->get_matrix(), DataFileType::crs);
            fd.close();
            string sort_cmd = "/usr/bin/sort -k1,1 -k2,2 -n -o" + output_file + " " + output_file;
            system(sort_cmd.c_str());
        }
    }
}

void Executor::buildMNCSketches(vector<TransitionMatrix*> matrices) {
    for (TransitionMatrix *m : matrices) {
        m->buildSketches();

        // m->print();
        //m->printSketches();
    }
}

void Executor::freeMatrices(algorithm_type algorithm, vector<TransitionMatrix*> matrices, TransitionMatrix *result, bool is_cached) {

    if (algorithm == algorithm_type::Seq || algorithm == algorithm_type::DynP) {
        delete result;

    // delete result if is not stored in cache
    } else if ( (algorithm == algorithm_type::OTreeA || algorithm == algorithm_type::Baseline1 
        || algorithm == algorithm_type::Baseline2) && !is_cached) {
        
        delete result;
    }

    // delete matrices
    for (TransitionMatrix* m : matrices) {
        if (m != nullptr)  {
            delete m;   
        }
    }
}

void Executor::freeMatrices() {
    auto it = this->_t_matrices.begin();

    while (it != this->_t_matrices.end()) {
        delete it->second;
        it++;
    }
}

void Executor::batch_run() {
//	 cout << "-------------------------------------" << endl;
    Utils::debug_msg("Reading constraints from file");
    //this->read_constraints();

    algorithm_type algorithm = this->_config->_algo;

    auto* hrank = new HRankSY(this->_config);
    auto* otree = new OTree(this->_config);
    auto* simpleBaseline = new SimpleBaseline(this->_config);
    auto* advBaseline = new AdvancedBaseline(this->_config);

    istream *infile = new ifstream(this->_config->_query_file);

    string query_line;
    vector<double> times;

    // execute all queries of query file
    while (getline(*infile, query_line)) {
	//  cout << "-------------------------------------" << endl;
        string metapath;
        tuple<string, string, string> constraint;
        int constraintIndex = -1;

        Utils::getMetapathAndConstraints(query_line, metapath, constraint);
        this->_config->setConstraint(constraint);

        Utils::debug_msg("Building constraint matrices for " + metapath);

        vector<int> dimensions;
        map<string, ConstraintMatrix*> constraint_matrices = buildConstraintMatrices(metapath, &dimensions);
        Utils::debug_msg("Building transition matrices");

        vector<unsigned long long int> non_zeros;
        vector<TransitionMatrix*> matrices = (this->_config->_adaptive)
             // TODO: should apply constraints in adaptive too
             ? buildTransitionMatricesAdaptive(metapath, dimensions, &non_zeros)
             : buildTransitionMatrices(metapath, dimensions, constraint_matrices, &constraintIndex);

        this->freeConstraintMatrices(constraint_matrices);

        // build MNC sketches, if required
        if (this->_config->getDynOptimizerType() == optimizer_type::MNC) {
            this->buildMNCSketches(matrices);
        }

        TransitionMatrix* result = nullptr;
        bool is_cached = false;

        clock_t begin = clock();

        if (algorithm == algorithm_type::Seq || algorithm == algorithm_type::DynP) {
            result = hrank->run(metapath, matrices, dimensions);
        } else if (algorithm == algorithm_type::OTreeA) {
            result = otree->run(metapath, matrices, dimensions, is_cached, constraintIndex);
        } else if (algorithm == algorithm_type::Baseline1) {
            result = simpleBaseline->run(query_line, matrices, dimensions, is_cached);
        } else if (algorithm == algorithm_type::Baseline2) {
            result = advBaseline->run(metapath, matrices, dimensions, is_cached);
        } else {
            cout << "ERROR: Unknown algorithm given" << endl;
            return;
        }

        // result->printNonZeros();
//         cout << "NON-ZEROS: " << result->get_matrix()->nonZeros() << endl;
//         cout << result->get_rows() << " x " << result->get_cols() << endl;
        // this->writeResults(result, metapath);

        double cur_time = double(clock() - begin) / CLOCKS_PER_SEC;
        times.push_back(cur_time);
        cout << ">\t" << metapath << " (" << get<0>(constraint) << "." << get<1>(constraint) << "=" << get<2>(constraint) << ")";

        cout << "\t" << cur_time << endl;

        this->freeMatrices(algorithm, matrices, result, is_cached);
    }

    cerr << accumulate(times.begin(), times.end(), 0.0);

    if (algorithm == algorithm_type::OTreeA) {
        cerr << "\t" << otree->getCacheSuccReads() << "\t" << otree->getCacheFailedReads() << "\t" << otree->getCacheWrites() << endl;
    } else if (algorithm == algorithm_type::Baseline1) {
        cerr << "\t" << simpleBaseline->getCacheSuccReads() << "\t" << simpleBaseline->getCacheFailedReads() << "\t" << simpleBaseline->getCacheWrites() << endl;
    } else if (algorithm == algorithm_type::Baseline2) {
        cerr << "\t" << advBaseline->getCacheSuccReads() << "\t" << advBaseline->getCacheFailedReads() << "\t" << advBaseline->getCacheWrites() << endl;
    }

    this->freeMatrices();
    delete hrank;
    delete otree;
    delete simpleBaseline;
    delete advBaseline;
    delete infile;
}
