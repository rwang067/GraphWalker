#define DYNAMIC_GRAPH

#include <string>
#include <fstream>
#include <cmath>
#include <fstream>

#include "api/graphwalker_basic_includes.hpp"
#include "engine/dynamicgraph.hpp"
#include "engine/importgraph.hpp"


int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line
     arguments and the configuration file. */
    set_argc(argc, argv);
    
    /* Metrics object for keeping track of performance count_invectorers
     and other information. Currently required. */
    metrics m("dynamic-graph-test");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    bid_t nverts = get_option_int("nverts", 4847571); // number of vertices
    uint16_t blocksize = get_option_int("blocksize", 64); // Size of block, represented in MB
    bid_t nblocks = get_option_int("nblocks", 5); // number of blocks
    size_t buffersize = get_option_int("buffersize", 2); // Size of edge buffer, represented in MB
    size_t logsize = get_option_int("logsize", 32); // Size of edge buffer, represented in MB

    m.set("file", filename);
    m.set("nverts", (size_t)nverts);
    m.set("nblocks", (size_t)nblocks);
    m.set("buffersize(MB)", (size_t)buffersize);
    m.set("logsize(MB)", (size_t)logsize);

    /* Detect the number of shards or preprocess an input to create them */
    // bid_t nblocks = convert_if_notexists(filename, blocksize);
    
    m.start_time("runtime");

    ImportGraph *importgraph = new ImportGraph();
    importgraph->clearDir(filename);
    importgraph->generateBlockRange(filename, blocksize, nblocks, nverts);

    DynamicGraph *graph = new DynamicGraph(filename, blocksize, nblocks, m, buffersize, logsize);
    m.start_time("importEdges");
    importgraph->importEdges(filename, graph);
    m.stop_time("importEdges");

    graph->addEdge(0, 56);
    graph->addEdge(1, 2);
    graph->addEdge(1, 110);
    std::vector<vid_t> neighbors = graph->getNeighbors(0);
    std::cout << "Got " << neighbors.size() << " neighbors of vertex 0 : " << std::endl;
    for(auto it = neighbors.begin(); it != neighbors.end(); it++)
        std::cout << *it << "\t";
    std::cout << std::endl;

    // graph->compaction(0);

    neighbors = graph->getNeighbors(1);
    std::cout << "Got " << neighbors.size() << " neighbors of vertex 1 : " << std::endl;
    for(auto it = neighbors.begin(); it != neighbors.end(); it++)
        std::cout << *it << "\t";
    std::cout << std::endl;

    neighbors = graph->getNeighbors(969513);
    std::cout << "Got " << neighbors.size() << " neighbors of vertex 969513 : " << std::endl;
    for(auto it = neighbors.begin(); it != neighbors.end(); it++)
        std::cout << *it << "\t";
    std::cout << std::endl;

    neighbors = graph->getNeighbors(4847570);
    std::cout << "Got " << neighbors.size() << " neighbors of vertex 4847570 : " << std::endl;
    for(auto it = neighbors.begin(); it != neighbors.end(); it++)
        std::cout << *it << "\t";
    std::cout << std::endl;

    m.stop_time("runtime");

    /* Report execution metrics */
    metrics_report(m);
    return 0;
}