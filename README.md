### Metapath-Based Pattern Mining in HINs

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

All methods use the same format of input files. Examples of input files can be found in the `./data/` folder. Three sample datasets are included: (a) a synthetic dataset in the folder `./data/synthetic_data/`, (b) a sparse sample of the DBLP dataset from AMiner (folder `./data/DBLP_sample_sparse/`) and (c) a denser sample from the same DBLP dataset (folder `./DBLP_sample_dense/`).



#### Clone & Build

```
# clone repository
git clone --recursive https://github.com/atrapos-hin/artifacts.git

# update submodules
git submodule update --remote

# build project
make
```

#### Execution

All the algorithms can be executed with the following command: 

```
./run -qf <query_file> -indir <input_node_directory> -idir <input_relations_directory> -c <constraints_file> -algo <algorithm> -ad <adaptive_mode> -mem <max_memory> -out <output_directory> -exp <otree_expand_type>
```

Parameters are explained in the table below:

| Parameter   |      Description      |
|----------|:-------------:|
| -qf |  the query file that contains all metapaths to be executed, each in a new line |
| -indir |    the directory that contains the input files for node attributes    |
| -idir | teh directory that contains the input files for node relations |
| -c | a file containing possible constraints for certian node types |
| -algo | the algorithm to be used; choose one of `Seq, DynP, OTree` |
| -ad | enable disk-based matrix multiplication, values: 0 or 1 |
| -mem | max memory to be used when the adapative (disk-based) matrix multiplcation is enabled |
| -out | the output directory |
| -exp | expansion strategy used in the OTree algorithm when trying to expand a previsously cached result. This parameter can be set to: `DynP`, `Seq`, `Exp` or `ExpSparse` |
| -dopt | heustistic to be used for Dynamic Programming matrix multiplication. This parameter can be set to: `Dense`, `Sparse`, `MNC` |
| -cache | cache size given in MB |


An example execution command can be the following: 
```
./run -qf data/synthetic_data/queries.txt -indir data/synthetic_data/nodes/ -idir data/synthetic_data/relations_dense/ -c data/constraints.txt -algo OTree -ad 0 -mem 3000 -out data/out/ -exp DynP -dopt Sparse -cache 1024
```

It executes the `OTree` algorithm with the `DynP` expansion strategy for the metapath queries in the file specified by the `-qf` flag and with the synthetic nodes and dense relations.  

#### References
[1] Y. P. S. W. B. Shi Chuan, Li Yitong. Constrained-meta-path-based rankingin heterogeneous information network. Knowledge and Information Systems, 2016
