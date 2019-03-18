INCFLAGS = -I/usr/local/include/ -I./src/ -I/usr/local/cuda/include -L/usr/local/cuda/lib64
CUDAFLAGS = -I/usr/local/cuda/include -I./src/ 

CPP = g++
# CPPFLAGS = -g -O0 $(INCFLAGS)  -fopenmp -Wall -Wno-strict-aliasing 
CPPFLAGS = -g -O3 $(INCFLAGS)  -fopenmp -Wall -Wno-strict-aliasing 
LINKERFLAGS = -lz
DEBUGFLAGS = -g -ggdb $(INCFLAGS)
HEADERS=$(shell find . -name '*.hpp')


apps : apps/rwdomination apps/graphlet apps/simrank apps/personalizedpagerank apps/msppr
 
echo:
	echo $(HEADERS)
clean:
	@rm -rf bin/*

# apps/% : apps/%.cpp $(HEADERS)
# 	@mkdir -p bin/$(@D)
# 	$(CPP) $(CPPFLAGS) -Iapp/ $@.cpp -o bin/$@ $(LINKERFLAGS)

apps/% : apps/%.cpp $(HEADERS) bin/cudacode.o
	@mkdir -p bin/$(@D)
	$(CPP) $(CPPFLAGS) -Iapp/ $@.cpp bin/cudacode.o -lcudart -o bin/$@ $(LINKERFLAGS)

bin/cudacode.o:
	nvcc $(CUDAFLAGS)  -c src/cuda/exec_update.cu -o bin/cudacode.o

# test pagerank
testp:
	make apps/pagerank
	./bin/apps/pagerank file "/home/wang/Documents/DataSet/Wikipedia/wikipedia_sorted.data" nvertices 12150977 nedges 378102402 nshards 5 R 10 L 10 tail 0

# test personalizedpagerank
testpp:
	make apps/personalizedpagerank
	./bin/apps/personalizedpagerank
