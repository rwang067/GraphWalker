#define DYNAMICEDATA 1
#include <string>
#include <fstream>
#include <vector>
#include <cmath>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/randomwalkwithrestart.hpp"

bool semi_external;

class Reachability : public RandomWalkwithRestart{
public:
    unsigned N, R, L;
    vid_t a,b;
    bool ans;

public:
	void initializeApp( vid_t _a, vid_t _b, unsigned _R, unsigned _L, float tail ){
		a = _a;
		b = _b;
		R = _R;
		L = _L;
		initializeRW( a, R, L, tail );
	}

	void startWalksbyApp(WalkManager &walk_manager){
		// R random walk start from a
		logstream(LOG_INFO) << "Start " << R << " walks of length " << L << " from a and b : " << std::endl;
		unsigned nthreads = get_option_int("execthreads", omp_get_max_threads());
		unsigned p = getInterval(a);
		vid_t cur = a - intervals[p].first;
		walk_manager.minstep[p] = 0;
		walk_manager.walknum[p] = R;

		unsigned cap = R/nthreads + 1;
		for( unsigned t = 0; t < nthreads; t++ )
			walk_manager.pwalks[t][p].reserve(cap);

        WalkDataType walk = walk_manager.encode(a, cur, 0);
		omp_set_num_threads(nthreads);
		#pragma omp parallel for schedule(static)
			for(unsigned i = 0; i < R; i++){
				walk_manager.pwalks[omp_get_thread_num()][p].push_back(walk);
			}
        if(!semi_external){
            //write to file
            walk_manager.freshIntervalWalks();
        }
    }

    void updateInfo(vid_t sourId, vid_t dstId, unsigned threadid, unsigned hop){
        // logstream(LOG_DEBUG) << "dstId : " << dstId << std::endl; 
        if( dstId == b )
            ans = true;
    }

	/**
     * Called before an execution interval is started.
     */
    void before_exec_interval(unsigned exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
		if(!semi_external){
			/*load walks*/
			walk_manager.readIntervalWalks(exec_interval);
		}
    }
    
    /**
     * Called after an execution interval has finished.
     */
    void after_exec_interval(unsigned exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
        walk_manager.walknum[exec_interval] = 0;
		walk_manager.minstep[exec_interval] = 0xfffffff;
        unsigned nthreads = get_option_int("execthreads");
        for(unsigned t = 0; t < nthreads; t++)
            walk_manager.pwalks[t][exec_interval].clear();
        for( unsigned p = 0; p < nshards; p++){
            if(p == exec_interval ) continue;
            if(semi_external) walk_manager.walknum[p] = 0;
			for(unsigned t=0;t<nthreads;t++){
				walk_manager.walknum[p] += walk_manager.pwalks[t][p].size();
            }
        }

        if(!semi_external){ 
			/*write back walks*/
			walk_manager.writeIntervalWalks(exec_interval);
		}
    }

};

int main(int argc, const char ** argv) {
    /* Read the command line arguments and the configuration file. */
    set_argc(argc, argv);  
    /* Metrics object for keeping track of performance count_invectorers and other information. Currently required. */
    metrics m("reachability");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file", "../DataSet/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    unsigned nvertices = get_option_int("nvertices", 4847571); // Number of vertices
    long long nedges = get_option_long("nedges", 68993773); // Number of edges
    unsigned nshards = get_option_int("nshards", 0); // Number of intervals
    unsigned a = get_option_int("a", 1); // vertex id of start source
    unsigned b = get_option_int("b", 2); // Number of sources
    unsigned R = get_option_int("R", 1000); // Number of steps
    unsigned L = get_option_int("L", 100); // Number of steps per walk
    float tail = get_option_float("tail", 0); // Ratio of stop long tail
    float prob = get_option_float("prob", 0.2); // prob of chose min step
    semi_external = get_option_int("semi_external", 0);
    
    /* Detect the number of shards or preprocess an input to create them */
    nshards = convert_if_notexists(filename, get_option_string("nshards", "auto"), nvertices, nedges, R, nshards);

    /* Run */
    Reachability program;
    program.initializeApp( a, b, R, L, tail );
    // program.initializeRW( 2*R, L, tail );
    graphwalker_engine engine(filename, nshards, m);
    engine.run(program, prob);

    std::cout << "Reachability from " << a << " to " << b << " = " << program.ans << std::endl;
    
    /* Report execution metrics */
    metrics_report(m);
    return 0;
}