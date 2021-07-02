#!/bin/bash

n="../../data/GDELT/nodes/"
r="../../data/GDELT/relations/"

logs=("q0" "q1" "q2" "q3" "q4")
cache_size=("512" "1024" "2048" "4096" "8128")
opts=("Sparse" "Dense" "MNC" )

for opt in "${opts[@]}"
do
	for l in "${logs[@]}"
	do
    		echo -e "\n"$l"\n"
		log="../../data/GDELT/vldb_queries/sessions_0.1//$l.csv"

    		for c in "${cache_size[@]}"
    		do
        		echo -e "$c"
			echo -n -e $opt"\t"$l"\t"$c"\t" >> "results/GDELT/overall.csv"
        		../../run -qf $log -indir $n -idir $r -c data/constraints.txt -algo OTree -ad 0 -mem 3000 -out data/out/ -exp DynP -dopt $opt -cache $c > results/"$opt"_"$l"_"$c".csv 2>>"results/GDELT/overall.csv"
    		done
	done
done
