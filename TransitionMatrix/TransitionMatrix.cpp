#include <iostream>
#include <fstream>
#include <ctime>
#include <unistd.h>
#ifdef __APPLE__
    #include <sys/wait.h>
#else
    #include <wait.h>
#endif
#include "TransitionMatrix.h"
#include "../ext_libs/Eigen/Sparse"
#include "../Utils.h"

using namespace std;
using namespace Eigen;

double TransitionMatrix::read() {
    clock_t begin = clock();

    //cout << _relationDir << " " << relation_ << " " << _rows << ", " << _cols << endl;

    _matrix = new SparseMatrix<int>(_rows, _cols);
    std::vector< Eigen::Triplet<int> > tripletList;

    ifstream infile(_crsFile);

    if (infile.good()) {
        string line;

        while (getline(infile, line)) {
            //cout << line << endl;

            int i = 0;
            int src, dest, val = 1;
            std::istringstream iss(line);

            while (getline(iss, line, '\t')) {
                if (i == 0) {
                    src = strtol(line.c_str(), nullptr, 10);
                } else if (i == 1){
                    dest = strtol(line.c_str(), nullptr, 10);
                } else if (i == 2) {
                    val = strtol(line.c_str(), nullptr, 10);
                }
                i++;
            }

            tripletList.emplace_back(src, dest, val);
        }

        _matrix->setFromTriplets(tripletList.begin(), tripletList.end());
        _matrix->makeCompressed();
    }

    infile.close();

    clock_t end = clock();

    _isInMemory = true;

    return double(end - begin) / CLOCKS_PER_SEC;
}

void TransitionMatrix::print() {
    cout << *_matrix << endl;
}

void TransitionMatrix::printNonZeros() {
    for (int k=0; k < this->_matrix->outerSize(); ++k) {
        for (SparseMatrix<int>::InnerIterator it((*this->_matrix), k); it; ++it) {
            cout << it.row() << ", " << it.col() << " = " << it.value() << endl;
        }
    }
}

Eigen::SparseMatrix<int>* TransitionMatrix::get_matrix() const {
    return _matrix;
}

unsigned long long int TransitionMatrix::non_zeros() {
    unsigned long long int count = 0;

    ifstream infile(_crsFile);

    if (infile.good()) {
        string line;
        while (getline(infile, line)) {
            count++;
        }
    }
    infile.close();
    return count;
}

const string &TransitionMatrix::get_relation() const {
    return _relation;
}

void TransitionMatrix::get_table_parts(TransitionMatrix *a, TransitionMatrix *b, int *a_part_rows, int *b_part_cols, int max_mem) {
    long double total_mem = 0;

    *a_part_rows = a->get_rows();
    *b_part_cols = b->get_cols();
    total_mem = estimate_mem(a, b, a_part_rows, b_part_cols);

    while(total_mem >= max_mem) {

        if (*a_part_rows > 1) {
            (*a_part_rows)--;
        }

        if (*b_part_cols > 1) {
            (*b_part_cols)--;
        }

        total_mem = estimate_mem(a, b, a_part_rows, b_part_cols);
    }
}

long double TransitionMatrix::estimate_mem(TransitionMatrix *a, TransitionMatrix *b, const int *a_part_rows, const int *b_part_cols) {

    unsigned long long int a_mem = (unsigned long long int)(*a_part_rows) * a->get_cols();
    a_mem *= sizeof(int);
    a_mem *= 3;

    //cout << "A: " << *a_part_rows << " x " << a->get_cols() << " = " << a_mem << endl;

    unsigned long long int b_mem = (unsigned long long int)(*b_part_cols) * b->get_rows();
    b_mem *= sizeof(int);
    b_mem *= 3;

    //cout << "B: " << *b_part_cols << " x " << b->get_rows() << " = " << b_mem << endl;

    unsigned long long int res_mem = (unsigned long long int)(*a_part_rows) * (*b_part_cols);
    res_mem *= sizeof(int);
    res_mem *= 3;

    //cout << "res: " << *a_part_rows << " x " << *b_part_cols << " = " << res_mem << endl;
    unsigned long long int total_mem = a_mem + b_mem + res_mem;

    //cout << "TOTAL: " << total_mem << endl;
    return total_mem / (1024.0 * 1024.0);
}

TransitionMatrix* TransitionMatrix::dot(TransitionMatrix *a, TransitionMatrix *b, string tmp_dir) {
    string res_relation = a->get_relation() + b->get_relation().substr(1);

    //cout << a->get_relation() << " x " << b->get_relation() << endl;
    auto *result = new SparseMatrix<int>();
    
    try {
        *result = (*(a->get_matrix()) * *(b->get_matrix()));       // multiply and prune zeros
    } catch (const exception& e) {
        cout << "You should not get there" << endl;
        cout << e.what() << endl;
    }

    return new TransitionMatrix(res_relation, tmp_dir, result, result->rows(), result->cols());
}

TransitionMatrix* TransitionMatrix::adaptive_dot(TransitionMatrix *a, TransitionMatrix *b,
                                                 unsigned long long int *reads, int max_mem, string tmp_dir) {

    int a_rows, b_cols;

    // get number of rows and columns to be used from each table
    get_table_parts(a, b, &a_rows, &b_cols, max_mem);

    // check if tables can be stored entirely in memory
    bool a_in_mem = (a_rows == a->get_rows());
    bool b_in_mem = (b_cols == b->get_cols());

    Utils::debug_msg(a->get_relation() + " x " + b->get_relation());
    Utils::debug_msg(a->get_relation() + " (" + to_string(a_rows) + " rows) - " + ((a_in_mem) ? "in memory" : "in disk"));
    Utils::debug_msg(b->get_relation() + " (" + to_string(b_cols) + " rows) - " + ((b_in_mem) ? "in memory" : "in disk"));

    string res_relation = a->get_relation() + b->get_relation().substr(1);

    // read tables from files and perform in-memory multiplication
    if (a_in_mem && b_in_mem) {

        if (!a->is_in_memory()) {
            a->read();
        }

        if (!b->is_in_memory()) {
            b->read();
        }

        auto *result = new Eigen::SparseMatrix<int>();
        *result = *(a->get_matrix()) * *(b->get_matrix());

        return new TransitionMatrix(res_relation, tmp_dir, result, result->rows(), result->cols());
    }

    // create object for result
    auto *res = new TransitionMatrix(res_relation, tmp_dir, a->get_rows(), b->get_cols(), false, false, false, false);

    a->prepare_crs_file();
    b->prepare_ccs_file();

    // open crs and ccs files of two tables
    ofstream res_fd = res->open_crs_file();
    ifstream a_crs_fd(a->get_crs_file());
    ifstream b_ccs_fd(b->get_ccs_file());

    // read rows in blocks from first table file
    while(Eigen::SparseMatrix<int> *rows = a->read_blocks(a_rows, a_crs_fd, SplitFileType::rows)) {
        //cout << *rows << endl;

        // seek to beginning of file
        b_ccs_fd.clear();
        b_ccs_fd.seekg(0, ios::beg);

        // read columns in blocks from second table file
        while(Eigen::SparseMatrix<int> *cols = b->read_blocks(b_cols, b_ccs_fd, SplitFileType::cols)) {
            //cout << *cols << endl;

            (*reads)++;

            auto *result = new Eigen::SparseMatrix<int>();

            // compute partial result and write it to crs file
            *result = *rows * *cols;
            delete cols;

            res->write_to_file(res_fd, result, DataFileType::crs);

            //Utils::print(result);
            delete result;
        }

        delete rows;
    }

    // close file descriptors
    b_ccs_fd.close();
    a_crs_fd.close();
    res_fd.close();

    return res;
}

void TransitionMatrix::prepare_crs_file() {

    if (!has_crs_file()) {

        // write from mem to crs file
        Utils::debug_msg("No CRS file exists");

        if (is_in_memory() && !has_ccs_file()) {

            Utils::debug_msg("Write CRS file from memory");

            ofstream fd = this->open_crs_file();
            this->write_to_file(fd, _matrix, DataFileType::crs);
            fd.close();

            // remove data from memory
            delete _matrix;
            _isInMemory = false;

        // convert ccs to ccs file
        } else if (has_ccs_file()) {

            Utils::debug_msg("Convert ccs file to crs");

            copy_files(this->get_ccs_file(), this->get_crs_file(), DataFileType::crs);
        } else {
            perror("Error preparing CRS file");
        }
    }

    if (!is_crs_sorted()) {
        Utils::debug_msg("CRS file is not sorted, sort it now");

        this->sort_crs_file();
    }
}

void TransitionMatrix::prepare_ccs_file() {
    if (!has_ccs_file()) {
        Utils::debug_msg("No CCS file exists");

        // if is only in memory
        if (is_in_memory() && !has_crs_file()) {

            Utils::debug_msg("Write CCS file from memory");

            ofstream fd = this->open_ccs_file();
            this->write_to_file(fd, _matrix, DataFileType::ccs);
            fd.close();

            // remove data from memory
            delete _matrix;
            _isInMemory = false;

        // convert crs to ccs file
        } else if (has_crs_file()) {

            Utils::debug_msg("Convert CRS file to CCS");
            copy_files(this->get_crs_file(), this->get_ccs_file(), DataFileType::ccs);

        } else {
            perror("Error preparing ccs file");
        }
    }

    if (!is_ccs_sorted()) {
        Utils::debug_msg("CCS file is not sorted, sort it now");
        this->sort_ccs_file();
    }
}

void TransitionMatrix::write_to_file(ofstream &fd, Eigen::SparseMatrix<int> *tmp_result, DataFileType type) {
    for (int k=0; k < (*tmp_result).outerSize(); ++k) {
        for (Eigen::SparseMatrix<int>::InnerIterator it(*tmp_result, k); it; ++it) {
            fd << it.row() << "\t" << it.col() << "\t" << it.value() << "\n";
        }
    }

    // mark that file was created
    if (type == DataFileType::crs) {
        _hasCRSFile = true;
    } else if (type == DataFileType::ccs) {
        _hasCCSFile = true;
    }
}

int TransitionMatrix::get_rows() const {
    return _rows;
}

int TransitionMatrix::get_cols() const {
    return _cols;
}

bool TransitionMatrix::is_in_memory() const {
    return _isInMemory;
}

Eigen::SparseMatrix<int>* TransitionMatrix::read_blocks(int num_rows, ifstream &fd, SplitFileType split) {

    auto *matrix = new Eigen::SparseMatrix<int>(_rows, _cols);
    std::vector< Eigen::Triplet<int> > triple_list;

    int count = 0;
    int prev_id = -1;

    if (fd.good()) {

        string line;

        // tracks the fd position of the line to be read
        streampos row_pos = fd.tellg();

        while (getline(fd, line)) {

            //cout << line << endl;
            int i = 0;
            int src, dest, val = 1;
            std::istringstream iss(line);

            while (getline(iss, line, '\t')) {

                if (i == 0) {
                    src = strtol(line.c_str(), nullptr, 10);

                    if ((split == SplitFileType::rows) && (prev_id != src)) {
                        prev_id = src;
                        count++;
                    }
                } else if (i == 1) {
                    dest = strtol(line.c_str(), nullptr, 10);
                    if ((split == SplitFileType::cols) && (prev_id != dest)) {
                        prev_id = dest;
                        count++;
                    }
                } else if (i == 2) {
                    val = strtol(line.c_str(), nullptr, 10);
                }
                i++;
            }

            if (count == num_rows + 1) {

                // we discard this line for this block,
                // so we rewind to the start of the line
                // to be read again
                fd.seekg(row_pos);
                break;
            }

            triple_list.emplace_back(src, dest, val);

            row_pos = fd.tellg();
        }

        matrix->setFromTriplets(triple_list.begin(), triple_list.end());
        matrix->makeCompressed();
    }

    // no non-zero elements found, reached end-of-file, so return nullptr
    if (matrix->nonZeros() == 0) {
        delete matrix;
        matrix = nullptr;
    }

    return matrix;
}

const string &TransitionMatrix::get_ccs_file() const {
    return _ccsFile;
}

const string &TransitionMatrix::get_crs_file() const {
    return _crsFile;
}

ofstream TransitionMatrix::open_crs_file() {
    //cout << crs_file_ << endl;
    return ofstream(_crsFile);
}

ofstream TransitionMatrix::open_ccs_file() {
    //cout << _ccsFile << endl;
    return ofstream(_ccsFile);
}

const string &TransitionMatrix::get_relation_dir() const {
    return _relationDir;
}

int TransitionMatrix::sort_crs_file() {
    string sort_cmd = "/usr/bin/sort -k1 -n -o" + _crsFile + " " + _crsFile;
    int ret = system(sort_cmd.c_str());
    _isCRSSorted = true;
    return ret;
}

int TransitionMatrix::sort_ccs_file() {
    string sort_cmd = "/usr/bin/sort -k2 -n -o" + _ccsFile + " " + _ccsFile;
    int ret = system(sort_cmd.c_str());
    _isCCSSorted = true;
    return ret;
}

bool TransitionMatrix::has_crs_file() const {
    return _hasCRSFile;
}

bool TransitionMatrix::has_ccs_file() const {
    return _hasCCSFile;
}

bool TransitionMatrix::is_crs_sorted() const {
    return _isCRSSorted;
}

bool TransitionMatrix::is_ccs_sorted() const {
    return _isCCSSorted;
}

int TransitionMatrix::copy_files(string src, string trg, DataFileType trg_type) {
    string cp_cmd = "cp " + src + " " + trg;
    int ret = system(cp_cmd.c_str());
    // mark that file was created

    if (trg_type == DataFileType::crs) {
        _hasCRSFile = true;
    } else if (trg_type == DataFileType::ccs) {
        _hasCCSFile = true;
    }

    return ret;
}

long double TransitionMatrix::getSparsity() {
    return this->_matrix->nonZeros() / ((long double) this->_cols * this->_rows);
}

void TransitionMatrix::copy(const TransitionMatrix &that) {
    _relation = that.get_relation();
    _crsFile = that.get_crs_file();
    _ccsFile = that.get_ccs_file();

    _hasCRSFile = that.has_crs_file();
    _hasCCSFile = that.has_ccs_file();


    _isCRSSorted = that.is_crs_sorted();
    _isCCSSorted = that.is_ccs_sorted();

    _isInMemory = that.is_in_memory();

    _relationDir = that.get_relation_dir();

    _rows = that.get_rows();
    _cols = that.get_cols();

    _matrix = new SparseMatrix<int>(*(that.get_matrix()));
}


void TransitionMatrix::buildSketches() {
    this->_sketches = new MNCSketches(this->_rows, this->_cols);

    for (int k=0; k < this->_matrix->outerSize(); ++k) {
        for (SparseMatrix<int>::InnerIterator it((*this->_matrix), k); it; ++it) {

            //cout << it.row() << ", " << it.col() << endl;

            this->_sketches->rowSketchInc(it.row());
            this->_sketches->colSketchInc(it.col());
        }
    }

    // build extended sketches
    // this->_sketches->allocExtended(this->_rows, this->_cols);
    // auto *rowSketch = this->_sketches->getRowSketch();
    // auto *colSketch = this->_sketches->getColSketch();

    // for (int k=0; k < this->_matrix->outerSize(); ++k) {
    //     for (SparseMatrix<int>::InnerIterator it((*this->_matrix), k); it; ++it) {

    //         if (colSketch->at(it.col()) == 1) {
    //             this->_sketches->extRowSketchInc(it.row());
    //         }

    //         if (rowSketch->at(it.row()) == 1) {
    //             this->_sketches->extColSketchInc(it.col());
    //         }
    //     }
    // }
}

/**
 * returns memory size of TransitionMatrix in MB
 */
long double TransitionMatrix::memory() {

    long double mSize = 0;
    
    // size of storing non-zero elements and inner-indices array
    mSize += this->_matrix->nonZeros() * 2 * sizeof(int);

    // size of storing outer-starts
    mSize += this->_matrix->outerSize() * sizeof(int);

    return mSize / (1024.0 * 1024.0);
}
