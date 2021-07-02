#!/bin/bash

sessions=("mix")
#logs=("q0" "q1" "q2" "q3" "q4" "q5" "q6" "q7" "q8" "q9")

for session in "${sessions[@]}"
do
	#for l in "${logs[@]}"
	for l in $(seq 0 9)
	do
		echo -e "\n"$session"\n"
		log="../../queries/GDELT/HRank-Neo4j//$l.csv"

		echo $log

		# RUN HRANK-SH
		#echo -e "$c"
		echo -n -e "neo4j\t"$l"\t" >> "results/overall.csv"
		python3 neo4j_exp.py $log > results/"neo4j"_"$l".csv 2>>"results/overall.csv"
		echo -n -e "\n" >> "results/overall.csv"

	done
done



