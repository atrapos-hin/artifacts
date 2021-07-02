#ifndef HRANK_CONSTRAINTMATRIX_H
#define HRANK_CONSTRAINTMATRIX_H

#include "../ext_libs/Eigen/Sparse"
#include <string>
#include <map>

using namespace std;
using  namespace Eigen;

typedef Eigen::Triplet<int> T;

class ConstraintMatrix {
private:
    string node_name_;
    string node_data_file_;
    // map<string, string> constraints_;
    int dimension_;

    SparseMatrix<int> *_matrix;
public:
    ConstraintMatrix(string node_name, string node_data_file, int dimension)
     : node_name_(node_name), node_data_file_ (node_data_file), dimension_ (dimension) {}

    ~ConstraintMatrix() { 
        delete this->_matrix; 
    }

    void build(map<string, string> *constraints_map);

    int get_dimension() const;

    void print();

    SparseMatrix<int>* get_matrix() const;
};


#endif //HRANK_CONSTRAINTMATRIX_H
