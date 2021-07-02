#include <iostream>
#include <math.h>
#include "MatrixMultiplier.h"
#include "DynamicOptimizer.h"

#include "../Utils.h"

using namespace std;

TransitionMatrix* MatrixMultiplier::expand(vector<TransitionMatrix*> &matrices, size_t subpath_pos, bool adaptive, int max_memory, bool delete_input) {
    TransitionMatrix *result = matrices[subpath_pos];
    TransitionMatrix *tmp_ptr = nullptr;

    unsigned long long int reads = 0;

    string tmp_dir = result->get_relation_dir() + "tmp/";

    // start expanding multiplications to the left of the cached subpath
    for (int i = subpath_pos - 1; i >= 0; i--) {

        //cout << i << matrices[i]->get_relation() << endl;
        if (adaptive) {
            result = TransitionMatrix::adaptive_dot(matrices[i], result, &reads, max_memory, tmp_dir);
        } else {
            result = TransitionMatrix::dot(matrices[i], result, tmp_dir);
        }

        if (delete_input) {
            delete tmp_ptr;
            tmp_ptr = nullptr;
            delete matrices[i];
            matrices[i] = nullptr;
        }

    }

    // then, expand multiplications to the right of the subpath result
    for (int i = subpath_pos + 1; i < matrices.size(); i++) {

        //cout << i << matrices[i]->get_relation() << endl;
        if (adaptive) {
            result = TransitionMatrix::adaptive_dot(result, matrices[i], &reads, max_memory, tmp_dir);
        } else {
            result = TransitionMatrix::dot(result, matrices[i], tmp_dir);
        }

        if (delete_input) {
            delete tmp_ptr;
            tmp_ptr = nullptr;
            delete matrices[i];
            matrices[i] = nullptr;
        }
    }

    return result;
}

TransitionMatrix* MatrixMultiplier::sequential(vector<TransitionMatrix*> &matrices, bool adaptive, int max_memory, bool delete_input) {
    TransitionMatrix *result = matrices[0];
    TransitionMatrix *tmp_ptr = nullptr;
    
    // mark first matrix as deleted as it is being deleted below via *tmp_ptr
    matrices[0] = nullptr;

    unsigned long long int reads = 0;

    string tmp_dir = result->get_relation_dir() + "tmp/";

    for (unsigned int i=1; i<matrices.size(); i++) {

        tmp_ptr = result;

        if (adaptive) {
            result = TransitionMatrix::adaptive_dot(result, matrices[i], &reads, max_memory, tmp_dir);
        } else {
            result = TransitionMatrix::dot(result, matrices[i], tmp_dir);
        }

        if (delete_input) {
            delete tmp_ptr;
            tmp_ptr = nullptr;
            delete matrices[i];
            matrices[i] = nullptr;
        }
    }

    return result;
}

TransitionMatrix* MatrixMultiplier::expandSparse(vector<TransitionMatrix*> &matrices, bool adaptive, int max_memory, bool delete_input) {

    // keep track of multiplied matrices
    int multiplications = matrices.size() - 1;

    unsigned int startPos = 0;
    long double minSparsity = numeric_limits<long double>::max();

    // compute sparsities of matrices && find matrix with min sparsity
    long double sparsities[matrices.size()];
    for (unsigned int i = 0; i < matrices.size(); i++) {
        sparsities[i] = matrices[i]->getSparsity();
        if (sparsities[i] < minSparsity) {
            minSparsity = sparsities[i];
            startPos = i;
        }
    }

    int leftPos = (startPos == 0) ? -1 : startPos - 1;
    int rightPos = (startPos == matrices.size() - 1) ? -1 : startPos + 1;

    TransitionMatrix *result = matrices[startPos];
    TransitionMatrix *tmp_ptr = nullptr;
    
    // mark first matrix as deleted as it is being deleted below via *tmp_ptr
    matrices[startPos] = nullptr;

    unsigned long long int reads = 0;

    string tmp_dir = result->get_relation_dir() + "tmp/";

    // starting from the matrix with the min sparsity, 
    // multiply it each time with the adjucent matrix with the min sparsity
    while(multiplications) {
        tmp_ptr = result;
       
        long double leftSparsity = numeric_limits<long double>::max();
        long double rightSparsity = numeric_limits<long double>::max();

        if (leftPos != -1) {
            leftSparsity = sparsities[leftPos];
        }

        if (rightPos != -1) {
            rightSparsity = sparsities[rightPos];
        }

        // multiply with left matrix
        if (leftSparsity < rightSparsity) {

            if (adaptive) {
                result = TransitionMatrix::adaptive_dot(matrices[leftPos], result, &reads, max_memory, tmp_dir);
            } else {
                result = TransitionMatrix::dot(matrices[leftPos], result, tmp_dir);
            }

            if (delete_input) {
                delete matrices[leftPos];
                matrices[leftPos] = nullptr;
                delete tmp_ptr;
                tmp_ptr = nullptr;
            }

            leftPos = (leftPos == 0) ? -1 : leftPos - 1;
        
        // multiply with right matrix
        } else if (rightSparsity < leftSparsity) {

            if (adaptive) {
                result = TransitionMatrix::adaptive_dot(result, matrices[rightPos], &reads, max_memory, tmp_dir);
            } else {
                result = TransitionMatrix::dot(result, matrices[rightPos], tmp_dir);
            }

            if (delete_input) {
                delete matrices[rightPos];
                matrices[rightPos] = nullptr;
                delete tmp_ptr;
                tmp_ptr = nullptr;
            }

            rightPos = (rightPos == matrices.size() - 1) ? -1 : rightPos + 1;
        } 

        multiplications--;
    }

    return result;
}


TransitionMatrix* MatrixMultiplier::dynamic(vector<TransitionMatrix*> &matrices, bool adaptive, int max_memory,
        vector<int> dimensions, optimizer_type sparse_optimizer, bool delete_input, long double &cost) {

    string tmp_dir = matrices[0]->get_relation_dir() + "tmp/";

    auto dynamic_optimizer = new DynamicOptimizer(dimensions.size());

    // clock_t begin = clock();

    // plan multiplications based on sparsity or only on matrix dimensions
    if (sparse_optimizer == optimizer_type::Sparse) {
        cost = dynamic_optimizer->sparse_optimal_matrix_chain_order(dimensions, matrices);
    } else if (sparse_optimizer == optimizer_type::Dense){
        dynamic_optimizer->optimal_matrix_chain_order(dimensions);
    } else if (sparse_optimizer == optimizer_type::MNC) {
        dynamic_optimizer->mnc_optimal_matrix_chain_order(dimensions, matrices);
    } else {
        cerr << "Error: Not valid dynamic optimizer given" << endl;
        return nullptr;
    }
    
    // double cur_time = double(clock() - begin) / CLOCKS_PER_SEC;
    // cout << "MNC-Dynamic: " << cur_time << endl;
    #ifdef DEBUG_MSG
        cout << "MULT ORDER: ";
        dynamic_optimizer->print_optimal_chain_order(0, dimensions.size() - 2);
        cout << endl;
        for (TransitionMatrix* m : matrices) {
            cout << m->get_relation() << endl;
        }
    #endif

    // get ordering of multiplications
    vector<pair<int, int>> chain_order;
    dynamic_optimizer->get_optimal_chain_order(0, dimensions.size() - 2, &chain_order); 

    vector<TransitionMatrix*> temp;
    TransitionMatrix *tmp_ptr = nullptr;

    unsigned long long int reads = 0;

    for (auto it = chain_order.begin(); it != chain_order.end(); ++it) {

        int k = it->first;
        int l = it->second;
        int n = temp.size();

        if (k >= 0 && l >= 0) {

            TransitionMatrix *res = (adaptive)
                        ? TransitionMatrix::adaptive_dot(matrices[k], matrices[l], &reads, max_memory, tmp_dir)
                        : TransitionMatrix::dot(matrices[k], matrices[l], tmp_dir);

            // delete multiplied matrices
            if (delete_input) {
                delete matrices[k];
                matrices[k] = nullptr;
                delete matrices[l];
                matrices[l] = nullptr;
            }

            temp.push_back(res);

        } else if (k == -1 && l >= 0) {

            tmp_ptr = temp[n-1];
            temp[n-1] = (adaptive)
                        ? TransitionMatrix::adaptive_dot(temp[n - 1], matrices[l], &reads, max_memory, tmp_dir)
                        : TransitionMatrix::dot(temp[n-1], matrices[l], tmp_dir);

            // delete multiplied matrices
            if (delete_input) {
                delete matrices[l];
                matrices[l] = nullptr;
                delete tmp_ptr;
                tmp_ptr = nullptr;
            }

        } else if (k >= 0 && l == -1) {

            tmp_ptr = temp[n-1];
            temp[n-1] = (adaptive)
                        ? TransitionMatrix::adaptive_dot(matrices[k], temp[n - 1], &reads, max_memory, tmp_dir)
                        : TransitionMatrix::dot(matrices[k], temp[n-1], tmp_dir);

            // delete multipliced matrices
            if (delete_input) {
                delete matrices[k];
                matrices[k] = nullptr;
                delete tmp_ptr;
                tmp_ptr = nullptr;
            }

        } else {

            tmp_ptr = temp[n-2];
            temp[n-2] = (adaptive)
                        ? TransitionMatrix::adaptive_dot(temp[n - 2], temp[n - 1], &reads, max_memory, tmp_dir)
                        : TransitionMatrix::dot(temp[n-2], temp[n-1], tmp_dir);

            // delete multiplied matrices
            if (delete_input) {
                delete tmp_ptr;
                tmp_ptr = nullptr;
                delete temp[n - 1];
                temp[n - 1] = nullptr;
            }

            temp.pop_back();
        }
    }

    delete dynamic_optimizer;

    return temp[0];
}

vector<string> MatrixMultiplier::getSubpathsFromDynamicPlanning(vector<TransitionMatrix*> matrices, vector<int> dimensions) {
    auto dynamic_optimizer = new DynamicOptimizer(dimensions.size());

    clock_t begin = clock();
    long double cost = dynamic_optimizer->sparse_optimal_matrix_chain_order(dimensions, matrices);

    // get ordering of multiplications
    vector<pair<int, int>> chain_order;
    dynamic_optimizer->get_optimal_chain_order(0, dimensions.size() - 2, &chain_order);

    vector<string> subpaths;

    for (auto it = chain_order.begin(); it != chain_order.end(); ++it) {

        int k = it->first;
        int l = it->second;
        int n = subpaths.size();

        if (k >= 0 && l >= 0) {
            subpaths.push_back(matrices[k]->get_relation() + matrices[l]->get_relation().substr(1));
        } else if (k == -1 && l >= 0) {
            subpaths.push_back(subpaths[n-1] + matrices[l]->get_relation().substr(1));
        } else if (k >= 0 && l == -1) {
            subpaths.push_back(matrices[k]->get_relation() + subpaths[n-1].substr(1));
        } else {
            subpaths.push_back(subpaths[n-2] + subpaths[n-1].substr(1));
        }
    }

    delete dynamic_optimizer;

    double cur_time = double(clock() - begin) / CLOCKS_PER_SEC;

cout << cur_time << "\t";
for (string s : subpaths) {
        cout << s << "-";
}
cout << "\t";


    return subpaths;
}

vector<string> MatrixMultiplier::getSubpathsFromDynamicPlanningWithCache
            (vector<TransitionMatrix*> matrices, 
            vector<int> dimensions, 
            string metapath,
            unordered_map<string, NodeInfo*> nodesWithSparsity, 
            tuple<string, string, string> constraint,
            int _s, 
            ST* stree
    ) {

    clock_t begin = clock();

    DynamicOptimizer *dynamic_optimizer = nullptr;

    auto cache_dopt = new DynamicOptimizer(dimensions.size());
    long double cache_plan_cost = cache_dopt->sparse_optimal_matrix_chain_order_from_cache(metapath, dimensions, matrices, nodesWithSparsity, constraint, _s, stree);

    auto dopt = new DynamicOptimizer(dimensions.size());
    long double simple_plan_cost = dopt->sparse_optimal_matrix_chain_order(dimensions, matrices);

    // cout << "SIMPLE: " << simple_plan_cost << "\t" << "CACHE: " << cache_plan_cost << endl;

    dynamic_optimizer = (cache_plan_cost < simple_plan_cost) ? cache_dopt : dopt;

    // get ordering of multiplications
    vector<pair<int, int>> chain_order;
    dynamic_optimizer->get_optimal_chain_order(0, dimensions.size() - 2, &chain_order);

    vector<string> subpaths;

    for (auto it = chain_order.begin(); it != chain_order.end(); ++it) {

        int k = it->first;
        int l = it->second;
        int n = subpaths.size();

        if (k >= 0 && l >= 0) {
            subpaths.push_back(matrices[k]->get_relation() + matrices[l]->get_relation().substr(1));
        } else if (k == -1 && l >= 0) {
            subpaths.push_back(subpaths[n-1] + matrices[l]->get_relation().substr(1));
        } else if (k >= 0 && l == -1) {
            subpaths.push_back(matrices[k]->get_relation() + subpaths[n-1].substr(1));
        } else {
            subpaths.push_back(subpaths[n-2] + subpaths[n-1].substr(1));
        }
    }

    delete cache_dopt;
    delete dopt;

    double cur_time = double(clock() - begin) / CLOCKS_PER_SEC;
/*
cout << cur_time << "\t";
for (string s : subpaths) {
	cout << s << "-";
}
cout << "\t";
*/
    return subpaths;
}

long double MatrixMultiplier::estimateResultMemory(vector<int> dims, vector<TransitionMatrix*> matrices) {
    return DynamicOptimizer::estimateResultMemory(dims, matrices);
}
