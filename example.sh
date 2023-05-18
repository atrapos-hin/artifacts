#!/bin/bash

n="./data/DBLP-0.6/nodes/"
r="./data/DBLP-0.6/relations/"

algorithms=("DynP" "Baseline1" "Baseline2" "OTree")
logs=("0" "1")
cache_size=("2048" "4096")
cache_policy=("LRU" "GDS" "PGDSU")
dynamic_optimizer=("Sparse")

echo -e "algorithm\tdynamic optimizer\tlog\tcache size\tcache policy"

for algo in "${algorithms[@]}"
do
	#echo -e "algorithms: ${algo}\n"

	for dopt in "${dynamic_optimizer[@]}"
	do
		#echo -e "dynamic optimizer: ${dopt}\n"

		for logname in "${logs[@]}"
		do
			#echo -e "log: ${logname}\n"

			log="./workloads/SCHOLARLY_HIN/CACHE_SIZE/${logname}.csv"

			for csize in "${cache_size[@]}"
			do

				#echo -e "cache size: ${csize}\n"

				for cachep in "${cache_policy[@]}"
				do
					echo -e "${algo}\t${dopt}\t${logname}\t${csize}\t${cachep}"

					echo -n -e ${algo}"\t"${dopt}"\t"${logname}"\t"${csize}"\t"${cachep}"\t" >> "results/overall.csv"
					./run -qf $log -indir $n -idir $r -algo $algo -ad 0 -mem 3000 -exp DynP -dopt $dopt -cache $csize -cachep $cachep > results/"${algo}"_"${dopt}"_"${logname}"_"${csize}"_"${cachep}".csv 2>>"results/overall.csv"
					echo -e  "\n" >> "results/overall.csv"
				done
			done
		done
	done
done
