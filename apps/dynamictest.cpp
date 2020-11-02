#define DYNAMIC_GRAPH

#include <string>
#include <fstream>
#include <cmath>
#include <fstream>

#include "api/graphwalker_basic_includes.hpp"
#include "engine/dynamicgraph.hpp"


int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line
     arguments and the configuration file. */
    set_argc(argc, argv);
    
    /* Metrics object for keeping track of performance count_invectorers
     and other information. Currently required. */
    metrics m("dynamic-graph-test");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    uint16_t blocksize = get_option_long("blocksize", 64); // Size of block, represented in MB
    bid_t nmblocks = get_option_int("nmblocks", 0); // number of in-memory blocks

    /* Detect the number of shards or preprocess an input to create them */
    bid_t nblocks = convert_if_notexists(filename, blocksize);

    DynamicGraph *graph = new DynamicGraph(filename, blocksize, nblocks, nmblocks);

    graph->addEdge(0, 56);
    graph->addEdge(1, 2);
    graph->addEdge(1, 110);
    std::vector<vid_t> neighbors = graph->getNeighbors(0);
    std::cout << "Got " << neighbors.size() << " neighbors of vertex 1 : " << std::endl;
    for(auto it = neighbors.begin(); it != neighbors.end(); it++)
        std::cout << *it << "\t";
    std::cout << std::endl;

    graph->compaction(0);

    neighbors = graph->getNeighbors(1);
    std::cout << "Got " << neighbors.size() << " neighbors of vertex 1 : " << std::endl;
    for(auto it = neighbors.begin(); it != neighbors.end(); it++)
        std::cout << *it << "\t";
    std::cout << std::endl;

    /* Report execution metrics */
    metrics_report(m);
    return 0;
}