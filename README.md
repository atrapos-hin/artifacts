### ATRAPOS: Metapath-Based Pattern Mining in HINs

> More details about BIP! Finder can be found in our publication:
> 
> S. Chatzopoulos, T. Vergoulis, D. Skoutas, T. Dalamagas, C. Tryfonopoulos, P. Karras. Atrapos: Real-time Evaluation of Metapath Query Workloads. The Web Conference (WWW), 2023
>
> -- We kindly ask that any published research that makes use of our work cites the paper above.


 

Many mining algorithms in HINs have a common computationally intensive step, that consists of finding the pairs of nodes connected given a particular metapath. This repository provides open-source implementations of the `Sequential` and `Dynamic Programming` matrix multiplication strategies proposed in [1] for this problem. 
As data scientists usually submit multiple metapath queries, we further propose the `OTree` algorithm that exploits metapath overlaps to eliminate redundant computations.

#### Input File Formats

Input files can be one of the following types:

* **File containing node attributes.** These are tab-separated files containing all node attributes. The first line is the header that contains all attribute names. The first column should be an incremental integer identifier, denoted as "id" in the header. These files should be named with the first letter of the entity they are representing. For example, the file that contains the attributes for node type `Author` should be named `A.csv`. An example of a file containing node attributes is the following: 

```
id	name    surname
0	Makoto  Satoh
1	Ryo Muramatsu
...
```

* **File containing node relations.** These are tab-separated files needed to construct the relations among nodes. These files contain two columns, the source and target identidiers respectively and should be sorted based on the first column and named with the extension `.crs`. They do not contain a header and they should be named according to the source and target node types. For example, the file with the relations between node types `Author` and `Paper` should be named `AP.crs`. An example of file containing node relations is the following: 

```
0	1
0	2
0	3
0	5
1	0
...
```

##### Example Input Files

All methods use the same format of input files. You can download sample data for the DBLP from [here](http://andrea.imsi.athenarc.gr/atrapos_sample_data.tar.gz); you have to extract the data into the `./data` folder.


#### Clone & Build

```
# clone repository
git clone https://github.com/atrapos-hin/artifacts.git

# navigate into the cloned folder
cd artifacts

# execute the following script from the root directory
bash setup.sh

# build project
make
```

#### Execution

All the algorithms can be executed with the following command: 

```
./run -qf [query_file] -indir [nodes_input_dir] -idir [relation_input_dir] -algo [algorithm] -ad [adaptive] -mem [max_mem_in_adaptive_mode] -exp [expansion_strategy] -dopt [heuristic_of_dynamic_programming] -cache [cache_size] -cachep [cache_policy]
```

Parameters are explained in the table below:

| Parameter   |      Description      |
|----------|:-------------:|
| -qf |  the query file that contains all metapaths to be executed, each in a new line |
| -indir |    the directory that contains the input files for node attributes    |
| -idir | teh directory that contains the input files for node relations |
| -algo | the algorithm to be used; choose one of: <ul><li>`Seq` for HRank (see [1]) using sequential matrix multiplication</li><li>`DynP` for HRank (see [1]) using the dynamic programming optimisation</li><li>`Baseline1` for CBS1 (caches only the result)</li><li>`CBS2` for CBS2 (caches final and intermediate results)</li><li>`OTree` for ATRAPOS (see [2])</li></ul></ul> |
| -ad | enable disk-based matrix multiplication, values: `0` (note that `1` is in experimental condition) |
| -mem | max memory to be used when the adapative (disk-based) matrix multiplcation is enabled, i.e, `-ad 1` is given |
| -exp | expansion strategy used in the OTree algorithm when trying to expand a previsously cached result. This parameter can be set to: `DynP` (also `Seq`, `Exp` and `ExpSparse` are in experimental condition) |
| -dopt | heustistic to be used for Dynamic Programming matrix multiplication. This parameter can be set to: `Dense`, `Sparse`, ( also `MNC` in experimental condition) |
| -cache | cache size given in MB |
| -cachep | the cache policy to be used; one of: <ul><li>`LRU` for the Least Recently Used</li><li>`GDS` for the GreedyDual-Size</li><li>`PGDSU` for the custom Popularity-aware GreedyDual-Size used in [2]</li></ul> |

You can use the `example.sh` script to execute a sample query workload for DBLP. 
It writes all output results under the `./results` folder and logs the overall time for each run in `./results/overall.csv`

#### References
[1] Y. P. S. W. B. Shi Chuan, Li Yitong. Constrained-meta-path-based rankingin heterogeneous information network. Knowledge and Information Systems, 2016

[2] S. Chatzopoulos, T. Vergoulis, D. Skoutas, T. Dalamagas, C. Tryfonopoulos, P. Karras. Atrapos: Real-time Evaluation of Metapath Query Workloads. The Web Conference (WWW), 2023
