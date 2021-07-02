#ifndef HRANK_TRANSITIONMATRIX_H
#define HRANK_TRANSITIONMATRIX_H

#include <string>
#include "../ext_libs/Eigen/Sparse"
#include "../MNCSketches/MNCSketches.h"

using namespace std;
using namespace Eigen;

typedef Triplet<int> T;

enum SplitFileType { rows, cols };
enum DataFileType { crs, ccs };

class TransitionMatrix {

private:
    string _relation;
    string _crsFile;
    string _ccsFile;

    bool _hasCRSFile { false };
    bool _hasCCSFile { false };

    bool _isCRSSorted { false };
    bool _isCCSSorted { false };

    bool _isInMemory { false };

    string _relationDir;

    int _rows { 0 };
    int _cols { 0 };

    SparseMatrix<int> *_matrix { nullptr };

    MNCSketches* _sketches { nullptr };

    static long double estimate_mem(TransitionMatrix *a, TransitionMatrix *b, const int *a_rows, const int *b_rows);
    static void get_table_parts(TransitionMatrix *a, TransitionMatrix *b, int *a_part_rows, int *b_part_cols, int max_mem);

public:
    explicit TransitionMatrix(string relation, const string& relation_dir, int rows, int cols,
            bool has_crs_file = true, bool has_ccs_file = true, bool crs_sorted = true, bool ccs_sorted = true) {

        _relation = relation;
        _relationDir = relation_dir;
        _crsFile = relation_dir + _relation + ".crs";
        _ccsFile= relation_dir + _relation + ".ccs";
        _rows = rows;
        _cols = cols;
        _hasCRSFile = has_crs_file;
        _hasCCSFile = has_ccs_file;
        _isCRSSorted = crs_sorted;
        _isCCSSorted = ccs_sorted;
    }

    TransitionMatrix() = default;

    TransitionMatrix(string relation, const string& relation_dir, SparseMatrix<int> *matrix, int rows, int cols) {
        _relation = relation;
        _relationDir = relation_dir;
        _crsFile = relation_dir + _relation + ".crs";
        _ccsFile= relation_dir + _relation + ".ccs";
        _matrix = matrix;
        _rows = rows;
        _cols = cols;
        _isInMemory = true;
    }

    ~TransitionMatrix() {
        delete _matrix;
        delete this->_sketches;
    }

    SparseMatrix<int>* get_matrix() const;
    unsigned long long int non_zeros();

    const string &get_relation() const;

    double read();
    void print();
    void printNonZeros();

    static TransitionMatrix* dot(TransitionMatrix *a, TransitionMatrix *b, string tmp_dir);
    static TransitionMatrix* adaptive_dot(TransitionMatrix *a, TransitionMatrix *b, unsigned long long *reads,
                                          int max_mem, string tmp_dir);

    int get_rows() const;

    int get_cols() const;

    const string &get_relation_dir() const;

    bool is_in_memory() const;

    SparseMatrix<int>* read_blocks(int num_rows, ifstream &fd, SplitFileType split);

    const string &get_ccs_file() const;

    const string &get_crs_file() const;

    ofstream open_crs_file();
    ofstream open_ccs_file();

    void write_to_file(ofstream &fd, SparseMatrix<int> *tmp_result, DataFileType fileType);
    int copy_files(string src, string trg, DataFileType trg_type);

    int sort_crs_file();
    int sort_ccs_file();

    bool has_crs_file() const;

    bool has_ccs_file() const;

    bool is_crs_sorted() const;

    bool is_ccs_sorted() const;

    void prepare_crs_file();

    void prepare_ccs_file();

    long double getSparsity();

    void copy(const TransitionMatrix &that);

    void buildSketches();

    vector<int>* getRowSketch() {
        return this->_sketches->getRowSketch();
    }

    vector<int>* getColSketch() {
        return this->_sketches->getColSketch();
    }

    void printSketches() {
        this->_sketches->print();
    }

    MNCSketches* getSketches() {
        return this->_sketches;
    }

    long double memory();
};




#endif //HRANK_TRANSITIONMATRIX_H
