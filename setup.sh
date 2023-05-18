#!/bin/bash

otree_repo_folder="ext_libs/GeneralizedSuffixTree"
otree_repo_branch="otree_constraints"

git clone https://github.com/vergoulis/GeSuTr.git ${otree_repo_folder}

cd ${otree_repo_folder}

git checkout ${otree_repo_branch}

cd ../..
