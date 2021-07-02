#ifndef HRANK_HRANKSY_H
#define HRANK_HRANKSY_H

#include "../Base/Config.h"
#include "../TransitionMatrix/TransitionMatrix.h"
#include "../TransitionMatrix/ConstraintMatrix.h"

class HRankSY {
private:
    Config* _config;

public:
    TransitionMatrix* run(string metapath, vector<TransitionMatrix*> &matrices, vector<int> dimensions);

    HRankSY(Config *config);
};


#endif //HRANK_HRANKSY_H
