CXX=g++
CXXFLAGS+=-Wall -O3 -std=c++14 -DNDEBUG

all: run

debug: CXXFLAGS += -g3 -DDEBUG_MSG
debug: run

run: main.o \
	Utils.o \
	Cache/LRUCache/LRUCache.o \
	Cache/GDSCache/GDSCache.o \
	Cache/GDSCache2/GDSCache2.o \
	Cache/GDSCache3/GDSCache3.o \
	Cache/GDSCache4/GDSCache4.o \
	Cache/PGDSUCache/PGDSUCache.o \
	Base/Config.o \
	HRank/HRankSY.o \
	TransitionMatrix/ConstraintMatrix.o \
	TransitionMatrix/TransitionMatrix.o \
	MNCSketches/MNCSketches.o \
	MatrixMultiplier/DynamicOptimizer.o \
	ext_libs/GeneralizedSuffixTree/ST.o \
	ext_libs/GeneralizedSuffixTree/STnode.o \
	Base/Executor.o \
	MatrixMultiplier/MatrixMultiplier.o \
	OTree/OTree.o \
	CacheBaselines/SimpleBaseline.o \
	CacheBaselines/AdvancedBaseline.o \

	$(CXX) -o $@ $^ -L BaseAlgorithm/ -L TransitionMatrix/ $(CXXFLAGS)

clean:
	rm *.o
	rm Base/*.o
	rm Cache/LRUCache/*.o
	rm Cache/GDSCache/*.o
	rm Cache/GDSCache2/*.o
	rm Cache/GDSCache3/*.o
	rm Cache/GDSCache4/*.o
	rm Cache/PGDSUCache/*.o
	rm HRank/*.o
	rm OTree/*.o
	rm TransitionMatrix/*.o
	rm MatrixMultiplier/*.o
	rm ext_libs/GeneralizedSuffixTree/*.o
	rm CacheBaselines/*.o
	rm run
