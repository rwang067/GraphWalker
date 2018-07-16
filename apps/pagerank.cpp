
#define DYNAMICEDATA 1

#include <string>
#include <fstream>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/simplerandomwalk.hpp"
#include "util/toplist.hpp"

int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line
     arguments and the configuration file. */
    set_argc(argc, argv);
    
    /* Metrics object for keeping track of performance count_invectorers
     and other information. Currently required. */
    metrics m("randomwalk");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file", "/home/wang/Documents/graph processing system/dataset/LiveJournal1/soc-LiveJournal1.txt");  // Base filename
    //int nvertices = get_option_int("nvertices"); // Number of vertices
    int nwalks = get_option_int("nwalks", 48475710); // Number of steps
    int nsteps = get_option_int("nsteps", 6); // Number of steps
    float tail = get_option_float("tail", 0.05); // Ratio of lower bound  of stop walks
    float prob = get_option_float("prob", 0.2); // Ratio of lower bound  of stop walks
    
    /* Detect the number of shards or preprocess an input to create them */
    int nshards = convert_if_notexists(filename, get_option_string("nshards", "auto"));

    /* Run */
    SimpleRandomWalk program;
    // RandomWalkwithRestartFixedstep program;
    program.initializeRW( nwalks, nsteps, tail );
    graphwalker_engine engine(filename, nshards, m);
    engine.run(program, prob);
    
    /* List top 20 */
    int ntop = 20;
    std::vector< vertex_value<VertexDataType> > top = get_top_vertices<VertexDataType>(filename, ntop);
    std::cout << "Print top 20 vertices: " << std::endl;
    for(int i=0; i < (int) top.size(); i++) {
        std::cout << (i+1) << ". " << top[i].vertex << "\t" << top[i].value << std::endl;
    }

    /* Report execution metrics */
    metrics_report(m);
    return 0;
}