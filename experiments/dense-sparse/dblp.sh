#!/bin/bash

n="../../data/DBLP_sample_sparse/nodes/"
r="../../data/DBLP_sample_sparse/relations/"

logs=("q0" "q1" "q2" "q3" "q4" "q5" "q6" "q7" "q8" "q9")
cache_size=("4096")
sessions=( "3" "4" "5" )

for session in "${sessions[@]}"
do
	for l in "${logs[@]}"
	do
		echo -e "\n"$session"\n"
		log="../../queries/DBLP/queries100/${session}/$l.csv"

		echo $log

		# RUN HRANK-SH
		#echo -e "$c"
		echo -n -e "HRankSH\t"$session"\t"$l"\t" >> "results/DBLP/overall.csv"
		../../run -qf $log -indir $n -idir $r -algo OTree -ad 0 -mem 3000 -out data/out/ -exp DynPB -dopt Sparse -cache 1024 -cachep LRU > results/DBLP/"HRank_SH"_"$l"_"$c".csv 2>>"results/DBLP/overall.csv"
		echo -n -e "\n" >> "results/DBLP/overall.csv"

	done
done
