#ifndef MPPAGERANK_SKETCHES_H
#define MPPAGERANK_SKETCHES_H

#include "../ext_libs/Eigen/Sparse"

using namespace Eigen;
using namespace std;

class MNCSketches {

    int _rows { 0 };
    int _cols { 0 };

    // sketch for rows
    vector<int> *_h_r { nullptr };
    vector<int> *_h_er { nullptr };

    // sketch for columns
    vector<int> *_h_c { nullptr };
    vector<int> *_h_ec { nullptr };


    int _h_r_nnz { 0 };
    int _h_c_nnz { 0 };

    int _max_h_r { 0 };
    int _max_h_c { 0 };

public:
    MNCSketches(int rows, int cols) {
        this->_rows = rows;
        this->_cols = cols;

        this->_h_r = new vector<int>(rows);
        this->_h_c = new vector<int>(cols);

    }

    void allocExtended(int rows, int cols) {
        this->_h_er = new vector<int>(rows);
        this->_h_ec = new vector<int>(cols);

    }

    void rowSketchInc(int i);
    void colSketchInc(int i);

    void extRowSketchInc(int i);
    void extColSketchInc(int i);

    static void printArr(vector<int> *arr);
    void print();

    vector<int>* getRowSketch() {
        return this->_h_r;
    }

    int getRowSketchNNZ() {
        return this->_h_r_nnz;
    }

    vector<int>* getColSketch() {
        return this->_h_c;
    }

    int getColSketchNNZ() {
        return this->_h_c_nnz;
    }

    vector<int>* getExtColSketch() {
        return this->_h_ec;
    }

    vector<int>* getExtRowSketch() {
        return this->_h_er;
    }

    ~MNCSketches() {
        delete this->_h_r;
        delete this->_h_c;

        delete this->_h_er;
        delete this->_h_ec;
    }

    int getMaxRowSketchValue() {
        return this->_max_h_r;
    }

    int getMaxColSketchValue() {
        return this->_max_h_c;
    }

    int rows() {
        return this->_rows;
    }

    int cols() {
        return this->_cols;
    }

    static long double getSparsity(MNCSketches* a_sketch, MNCSketches* b_sketch, int m, int n, int l);
    static long double densityMap(vector<int> *a, vector<int> *b, long double p);
    static long double dotSparsity(long double aSparsity, long double bSparsity, int commonDimension);
    static long double sumSparsity(long double aDensity, long double bDensity);
    static int countLargerThan(vector<int> *v, int value);
    static int countEqualToOne(vector<int> *v);
    static MNCSketches* propagate(MNCSketches *a, MNCSketches *b, int m, int n, int l);

    void propagateRowSketch(vector<int> *v, double sparsity, int m, int l);
    void propagateColSketch(vector<int> *v, double sparsity, int m, int l);

    static long double dot(vector<int> *a, vector<int> *b);
    static long double segmentNNZ(vector<int> *arr, int start, int len);
    static long double sum(vector<int> *v);
};


#endif //MPPAGERANK_SKETCHES_H
