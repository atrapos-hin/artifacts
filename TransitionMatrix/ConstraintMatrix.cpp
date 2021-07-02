#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>

#include "ConstraintMatrix.h"
#include "../Utils.h"
#include "../ext_libs/Eigen/Sparse"

using namespace std;
using namespace Eigen;

void ConstraintMatrix::build(map<string, string> *constraints_map) {

    Utils::debug_msg("Constraint matrix:" + node_name_);

    // create constraint matrix
    _matrix = new SparseMatrix<int>(dimension_, dimension_);

    std::vector<Triplet<int>> triplet_list;

    map<int, string> field_constraints;

    // map constraints to column indexes
    for(auto it = constraints_map->cbegin(); it != constraints_map->cend(); ++it) {
        int column_idx = Utils::get_column_idx(node_data_file_, it->first);
        field_constraints.insert(make_pair(column_idx, it->second));
    }

    int id_idx = Utils::get_column_idx(node_data_file_, "id");

    ifstream infile(node_data_file_);

    if (infile.good()) {
        string line;

        while (getline(infile, line)) {

            bool valid = true;

            int i = 0;
            int id = -1;
            std::istringstream iss(line);
            while (getline(iss, line, '\t')) {
                if (id_idx == i) {
                    id = strtol(line.c_str(), nullptr, 10);
                }

                auto it = field_constraints.find(i);
                if (it != field_constraints.end()) {
                    if (it->second.compare(line)) {
                        valid = false;
                        break;
                    }
                }
                i++;

            }

            if (valid) {
                triplet_list.emplace_back(id, id, 1);
                break; // assumes that only one node satisfies constraint, COMMENT OUT OTHERWISE
            }
        }

        _matrix->setFromTriplets(triplet_list.begin(), triplet_list.end());
    }

    infile.close();
}

int ConstraintMatrix::get_dimension() const {
    return dimension_;
}

SparseMatrix<int>* ConstraintMatrix::get_matrix() const {
    return _matrix;
}

void ConstraintMatrix::print() {
    for (int k=0; k < (*_matrix).outerSize(); ++k) {
        for (Eigen::SparseMatrix<int>::InnerIterator it(*_matrix, k); it; ++it) {
            std::cout << "(" << it.row() << ","; // row index
            std::cout << it.col() << ")\t"; // col index (here it is equal to k)
        }
    }
}
