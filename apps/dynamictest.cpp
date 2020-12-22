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
    for(auto it = neighbors.begin(); it != neighbors.end(); it++)
        std::cout << *it << "\t";
    std::cout << std::endl;
    return ;
}

void traverseGraph(DynamicGraph *graph){
    bid_t nblocks = graph->nblocks;
    eid_t edgecount = 0;
    for(bid_t p = 0; p < nblocks; p++){
        vid_t nverts = 0, *csr = NULL;
        eid_t nedges = 0, *beg_pos = NULL;
        graph->loadSubGraph(p, beg_pos, csr, &nverts, &nedges);
        edgecount += nedges;
        // std::cout << "p = " << p << ", s = " << s << ", nverts = " << nverts << ", nedges = " << nedges << std::endl;
        if(beg_pos != NULL) free(beg_pos);
        if(csr != NULL) free(csr);
    }
    std::cout << "edgecount = " << edgecount << std::endl;
}


int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line
     arguments and the configuration file. */
    set_argc(argc, argv);
    
    /* Metrics object for keeping track of performance count_invectorers
     and other information. Currently required. */
    metrics m("dynamic-graph-test");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/Friendster/out.friendster");  // Base filename
    // vid_t N = get_option_int("N", 68349467); // number of vertices
    size_t buffersize = get_option_int("buffersize", 64); // Size of edge buffer, represented in MB
    vid_t nverts_per_grp = get_option_int("nverts_per_grp", 16*1024); // number of vertices per log group
    size_t logsize = get_option_int("logsize", 2048); // Size of edge buffer, represented in KB
    size_t blocksize = get_option_int("blocksize", 32); // Size of block, represented in MB

    m.set("file", filename);
    m.set("nverts_per_grp", (size_t)nverts_per_grp);
    m.set("buffersize(MB)", buffersize);
    m.set("logsize(KB)", logsize);
    m.set("blocksize(MB)", blocksize);

    /* Detect the number of shards or preprocess an input to create them */
    // bid_t nblocks = convert_if_notexists(filename, blocksize);
    
    m.start_time("runtime");

    ImportGraph *importgraph = new ImportGraph(m);
    importgraph->clearDir(filename);
    // importgraph->generateBlockRange(filename, N, nverts_per_blk);

    m.start_time("createGraph");
    DynamicGraph *graph = new DynamicGraph(m, filename, blocksize, buffersize, nverts_per_grp, logsize);
    m.stop_time("createGraph");
    m.start_time("importEdges");
    importgraph->importEdgeList(filename, graph);
    m.stop_time("importEdges");

    m.set("nvertices", (size_t)(graph->N));
    m.set("ngroups", (size_t)(graph->ngroups));
    m.set("nblocks", (size_t)(graph->nblocks));


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
    searchNeighbor(graph, 0); // for FS, 0+0+0
    searchNeighbor(graph, 1); // for FS, 20+0+0
    searchNeighbor(graph, 12); // for FS, 16+0+0
    searchNeighbor(graph, 22); // for FS, 4+0+0
    searchNeighbor(graph, 68349394); // for FS, 0+1+1

    m.start_time("test_query");
    for(int i = 0; i < 10000; i++){
        vid_t v = rand() % graph->N;
        m.start_time("test_searchNeighbors");
        // searchNeighbor(graph, v);
        graph->getNeighbors(v);
        m.stop_time("test_searchNeighbors");
    }
    m.stop_time("test_query");

    m.start_time("test_traverse");
    traverseGraph(graph);
    m.stop_time("test_traverse");

    m.stop_time("runtime");

    /* Report execution metrics */
    metrics_report(m);
    return 0;
}