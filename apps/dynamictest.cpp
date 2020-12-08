#define DYNAMIC_GRAPH

#include <string>
#include <fstream>
#include <cmath>
#include <fstream>

#include "api/graphwalker_basic_includes.hpp"
#include "engine/dynamicgraph.hpp"
#include "engine/importgraph.hpp"

void searchNeighbor(DynamicGraph *graph, vid_t v){
    std::vector<vid_t> neighbors = graph->getNeighbors(v);
    std::cout << "Got " << neighbors.size() << " neighbors of vertex " << v << std::endl;
    // for(auto it = neighbors.begin(); it != neighbors.end(); it++)
    //     std::cout << *it << "\t";
    // std::cout << std::endl;
    return ;
}


int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line
     arguments and the configuration file. */
    set_argc(argc, argv);
    
    /* Metrics object for keeping track of performance count_invectorers
     and other information. Currently required. */
    metrics m("dynamic-graph-test");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    vid_t N = get_option_int("N", 4847571); // number of vertices
    vid_t nverts_per_blk = get_option_int("nverts_per_blk", 2*1024*1024); // number of vertices per block
    size_t buffersize = get_option_int("buffersize", 2); // Size of edge buffer, represented in MB
    size_t logsize = get_option_int("logsize", 32); // Size of edge buffer, represented in MB
    size_t segsize = get_option_int("segsize", 64); // Size of block, represented in MB

    m.set("file", filename);
    m.set("N", (size_t)N);
    m.set("nverts_per_blk", (size_t)nverts_per_blk);
    m.set("buffersize(MB)", buffersize);
    m.set("logsize(MB)", logsize);
    m.set("segsize(MB)", segsize);

    /* Detect the number of shards or preprocess an input to create them */
    // bid_t nblocks = convert_if_notexists(filename, blocksize);
    
    m.start_time("runtime");

    ImportGraph *importgraph = new ImportGraph(m);
    importgraph->clearDir(filename);
    importgraph->generateBlockRange(filename, N, nverts_per_blk);

    DynamicGraph *graph = new DynamicGraph(m, filename, N, nverts_per_blk, buffersize, logsize, segsize);
    m.start_time("importEdges");
    importgraph->importEdgeList(filename, graph);
    m.stop_time("importEdges");


    // //Test for LiveJournal
    // graph->addEdge(0, 51);
    // graph->flush();
    // graph->addEdge(0, 101);
    // searchNeighbor(graph, 0); // for LJ, 46+1+1
    // graph->addEdge(1, 11);
    // searchNeighbor(graph, 1); // for LJ, 194+1+0
    // graph->addEdge(4847570, 12);
    // searchNeighbor(graph, 4847570); // for LJ, 0+18+1

    //Test for Friendster
    graph->addEdge(68349394, 1100);
    searchNeighbor(graph, 12); // for FS, 16+0+0
    searchNeighbor(graph, 68349394); // for FS, 0+1+1

    m.start_time("test_query");
    for(int i = 0; i <= 10000; i++){
        vid_t v = rand() % N;
        m.start_time("test_searchNeighbors");
        searchNeighbor(graph, v);
        m.stop_time("test_searchNeighbors");
    }
    m.stop_time("test_query");

    m.stop_time("runtime");

    /* Report execution metrics */
    metrics_report(m);
    return 0;
}