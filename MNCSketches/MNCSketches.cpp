#include <iostream>
#include <math.h>
#include <numeric>
#include "MNCSketches.h"

using namespace std;

void MNCSketches::rowSketchInc(int i) {
    this->_h_r->at(i)++;

    int cur = this->_h_r->at(i);
    if (cur == 1)
        this->_h_r_nnz++;

    if (cur > this->_max_h_r)
        this->_max_h_r = cur;
}

void MNCSketches::colSketchInc(int i) {
    this->_h_c->at(i)++;

    int cur = this->_h_c->at(i);
    if (cur == 1)
        this->_h_c_nnz++;

    if (cur > this->_max_h_c)
        this->_max_h_c = cur;
}

void MNCSketches::extRowSketchInc(int i) {
    this->_h_er->at(i)++;
}

void MNCSketches::extColSketchInc(int i) {
    this->_h_ec->at(i)++;
}

void MNCSketches::printArr(vector<int> *arr) {
    for (unsigned int i = 0; i<arr->size(); i++) {
        cout << arr->at(i) << endl;
    }

}
void MNCSketches::print() {
    cout << "ROW SKETCH:" << endl;
    MNCSketches::printArr(this->_h_r);

    cout << "COL SKETCH:" << endl;
    MNCSketches::printArr(this->_h_c);

    if (this->_h_er) {
        cout << "ROW EXT SKETCH:" << endl;
        MNCSketches::printArr(this->_h_er);
    }

    if (this->_h_ec) {
        cout << "COL EXT SKETCH:" << endl;
        MNCSketches::printArr(this->_h_ec);
    }

    cout << this->_h_r_nnz << " kai " << this->_h_c_nnz << endl;

}

long double MNCSketches::dotSparsity(long double aDensity, long double bDensity, int commonDimension) {
    return (1 - pow(1 - aDensity * bDensity, commonDimension));
}

long double MNCSketches::sumSparsity(long double aDensity, long double bDensity) {
    return aDensity + bDensity - (aDensity * bDensity);
}

long double MNCSketches::segmentNNZ(vector<int> *arr, int start, int len) {
    long double nnz = 0;
    for (unsigned int i = start; i < start + len; i++) {
        if (arr->at(i) != 0){
            nnz++;
        }
    }

    return nnz;
}

long double MNCSketches::densityMap(vector<int> *a, vector<int> *b, long double p) {

    size_t len = a->size();
    size_t dMapLen = ceil(len / p);
    int step = (p > len) ? len : p;

    // construct density maps
    long double aDMap [dMapLen];
    long double bDMap [dMapLen];

    size_t start = 0, end = step;

    for (unsigned int i=0; i<dMapLen; i++) {

        int vLen = end - start;

        // density-map of first vector
        aDMap[i] = (long double) MNCSketches::segmentNNZ(a, start, vLen) / (long double)vLen;

        // density-map of second vector
        bDMap[i] = (long double) MNCSketches::segmentNNZ(b, start, vLen) / (long double)vLen;

        // adjust limits for segments
        start += step;
        end = (end + step > len) ? len : end + step;
    }

    long double density = 0;

    for (unsigned int i=0; i<dMapLen; i++) {
        long double curDensity = MNCSketches::dotSparsity(aDMap[i], bDMap[i], 1);
        density = MNCSketches::sumSparsity(density, curDensity);
    }

    return density;
}

int MNCSketches::countLargerThan(vector<int> *v, int value) {
    int count = 0;

    for (unsigned int i = 0; i<v->size(); i++) {

        if (v->at(i) > value) {
            count++;
        }
    }

    return count;
}

int MNCSketches::countEqualToOne(vector<int> *v) {
    int count = 0;

    for (unsigned int i = 0; i<v->size(); i++) {

        if (v->at(i) == 1) {
            count++;
        }
    }

    return count;
}


long double MNCSketches::dot(vector<int> *a, vector<int> *b) {
    long double sum = 0;

    for (unsigned int i=0; i<a->size(); i++) {
        sum += a->at(i) * b->at(i);
    }

    return sum;
}

long double MNCSketches::getSparsity(MNCSketches* a, MNCSketches* b, int m, int n, int l) {
    long double nnz = 0;

    if (a->getMaxRowSketchValue() <= 1 && b->getMaxColSketchValue() <= 1) {
        nnz = MNCSketches::dot(a->getColSketch(), b->getRowSketch());
    //} else if (a->getExtRowSketch() && b->getExtRowSketch()) {
    //    //nnz = a->getExtColSketch()->dot(*b->getRowSketch()) + (*a->getColSketch() - *a->getExtColSketch()).dot(*b->getExtRowSketch());
    //    ////nnz = MNCSketches::dot(a->getExtColSketch(), b->getRowSketch(), b->rows());
    //    //double p = (a->getRowSketch()->nonZeros() - MNCSketches::countEqualToOne(a->getRowSketch())) * (b->getColSketch()->nonZeros() - MNCSketches::countEqualToOne(b->getColSketch()));
    //    //SparseVector<int> aCols = *a->getColSketch() - *a->getExtColSketch();
    //    //SparseVector<int> bRows = *b->getRowSketch() - *b->getExtRowSketch();
    //    //nnz += MNCSketches::densityMap(&aCols, &bRows, p) * p;
    } else {
        long double p = a->getRowSketchNNZ() * b->getColSketchNNZ();
        nnz = MNCSketches::densityMap(a->getColSketch(), b->getRowSketch(), p) * p;
    }

    long double largerThanCount = MNCSketches::countLargerThan(a->getRowSketch(), n / 2)
            * MNCSketches::countLargerThan(b->getColSketch(), n / 2);

    nnz = max(nnz, largerThanCount);

    return ((long double) ((nnz) / (long double) m) * 1 / (long double) l);
}

long double MNCSketches::sum(vector<int> *v) {
    long double sum = 0;

    for (auto& n : *v)
        sum += n;

    return sum;
}

void MNCSketches::propagateRowSketch(vector<int> *v, double sparsity, int m, int l) {

   long double sum = MNCSketches::sum(v);

    for (unsigned int i = 0; i < v->size(); i++) {
        int temp = (int) round(v->at(i) * sparsity * m * l / sum);

        if (temp) {
            this->_h_r->at(i) = temp;
            this->_h_r_nnz++;
            if (temp > this->_max_h_r) {
                this->_max_h_r = temp;
            }
        }
    }
}

void MNCSketches::propagateColSketch(vector<int> *v, double sparsity, int m, int l) {

   long double sum = MNCSketches::sum(v);

    for (unsigned int i = 0; i < v->size(); i++) {

        int temp = (int) round(v->at(i) * sparsity * m * l / sum);

        if (temp) {
            this->_h_c->at(i) = temp;
            this->_h_c_nnz++;
            if (temp > this->_max_h_c) {
                this->_max_h_c = temp;
            }
        }
   }
}

MNCSketches* MNCSketches::propagate(MNCSketches *a, MNCSketches *b, int m, int n, int l) {
    long double resultSparsity = MNCSketches::getSparsity(a, b, m, n, l);

    auto *resultSketches = new MNCSketches(m, l);
    resultSketches->propagateRowSketch(a->getRowSketch(), resultSparsity, m, l);
    resultSketches->propagateColSketch(b->getColSketch(), resultSparsity, m, l);
    return resultSketches;
}

