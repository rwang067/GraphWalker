INCFLAGS = -I/usr/local/include/ -I./src/

CPP = g++
# CPPFLAGS = -g -O0 $(INCFLAGS)  -fopenmp -Wall -Wno-strict-aliasing 
CPPFLAGS = -g -O3 $(INCFLAGS)  -fopenmp -Wall -Wno-strict-aliasing 
LINKERFLAGS = -lz
DEBUGFLAGS = -g -ggdb $(INCFLAGS)
HEADERS=$(shell find . -name '*.hpp')


apps : apps/pagerank apps/rwdomination apps/graphlet apps/personalizedpagerank apps/simrank apps/reachability

echo:
	echo $(HEADERS)
clean:
	@rm -rf bin/*
	# cd toolkits/collaborative_filtering/; make clean; cd ../../
	# cd toolkits/parsers/; make clean; cd ../../
	# cd toolkits/graph_analytics/; make clean; cd ../../

blocksplitter: src/preprocessing/blocksplitter.cpp $(HEADERS)
	$(CPP) $(CPPFLAGS) src/preprocessing/blocksplitter.cpp -o bin/blocksplitter $(LINKERFLAGS)

sharder_basic: src/preprocessing/sharder_basic.cpp $(HEADERS)
	@mkdir -p bin
	$(CPP) $(CPPFLAGS) src/preprocessing/sharder_basic.cpp -o bin/sharder_basic $(LINKERFLAGS)

apps/% : apps/%.cpp $(HEADERS)
	@mkdir -p bin/$(@D)
	$(CPP) $(CPPFLAGS) -Iapp/ $@.cpp -o bin/$@ $(LINKERFLAGS) 


graphlab_als: example_apps/matrix_factorization/graphlab_gas/als_graphlab.cpp
	$(CPP) $(CPPFLAGS) example_apps/matrix_factorization/graphlab_gas/als_graphlab.cpp -o bin/graphlab_als $(LINKERFLAGS)

cf:
	cd toolkits/collaborative_filtering/; bash ./test_eigen.sh; 
	if [ $$? -ne 0 ]; then exit 1; fi
	cd toolkits/collaborative_filtering/; make 
cf_test:
	cd toolkits/collaborative_filtering/; make test; 
cfd:
	cd toolkits/collaborative_filtering/; make -f Makefile.debug

parsers:
	cd toolkits/parsers/; make
parsersd:
	cd toolkits/parsers/; make -f Makefile.debug
ga:
	cd toolkits/graph_analytics/; make
ta:
	cd toolkits/text_analysis/; make

docs: */**
	doxygen conf/doxygen/doxygen.config

# test pagerank
testp:
	make apps/pagerank
	# ./bin/apps/pagerank
	# ./bin/apps/pagerank file "/home/wang/Documents/DataSet/Yahoo/yahoo-webmap.txt" nvertices 1413511390 nedges 6636600779 R 1 L 1
	./bin/apps/pagerank file "/home/wang/Documents/DataSet/Wikipedia/wikipedia_sorted.data" nvertices 12150977 nedges 378102402 nshards 5 R 10 L 10 tail 0

# test personalizedpagerank
testpp:
	make apps/personalizedpagerank
	./bin/apps/personalizedpagerank
	

	
