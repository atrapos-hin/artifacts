#!/bin/bash

n="../../data/DBLP_full/nodes/"
r="../../data/DBLP_full/relations/"

logs=("q0" "q1" "q2" "q3" "q4")
cache_size=("512" "1024" "2048" "4096" "8128" "16256")
opts=("Sparse" "Dense" "MNC" )

#for opt in "${opts[@]}"
#do
#	for l in "${logs[@]}"
#	do
#		echo -e "\n"$l"\n"
#		log="../../data/DBLP_full/vldb_queries/sessions_0.1/$l.csv"
#
#		for c in "${cache_size[@]}"
#		do
#			echo -e "$c"
#		echo -n -e $opt"\t"$l"\t"$c"\t" >> "results/overall.csv"
#			../../run -qf $log -indir $n -idir $r -c data/constraints.txt -algo OTree -ad 0 -mem 3000 -out data/out/ -exp DynP -dopt $opt -cache $c > results/"$opt"_"$l"_"$c".csv 2>>"results/overall.csv"
#		done
#	done
#done

#for l in "${logs[@]}"
#do
#	echo -e "\n"$l"\n"
#	log="../../data/DBLP_full/vldb_queries/sessions_0.1/$l.csv"
#
#	for c in "${cache_size[@]}"
#	do
#		echo -e "$c"
#		echo -n -e "SIMPLE\t"$l"\t"$c"\t" >> "results/DBLP/overall.csv"
#		../../run -qf $log -indir $n -idir $r -c data/constraints.txt -algo OTree -ad 0 -mem 3000 -out data/out/ -exp ExpSparse -dopt Sparse -cache $c > results/DBLP/"SIMPLE"_"$l"_"$c".csv 2>>"results/DBLP/overall.csv"
#	done
#done

for l in "${logs[@]}"
do
       echo -e "\n"$l"\n"
       log="../../data/DBLP_full/vldb_queries/sessions_0.1/$l.csv"

       for c in "${cache_size[@]}"
       do
               echo -e "$c"
               echo -n -e "BASELINE1\t"$l"\t"$c"\t" >> "results/DBLP/overall.csv"
               ../../run -qf $log -indir $n -idir $r -c data/constraints.txt -algo Baseline1 -ad 0 -mem 3000 -out data/out/ -exp DynP -dopt Sparse -cache $c > results/DBLP/"BASELINE1"_"$l"_"$c".csv 2>>"results/DBLP/overall.csv"
       done
done
