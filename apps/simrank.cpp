
#define DYNAMICEDATA 1

#include <string>
#include <fstream>
#include <vector>
#include <cmath>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/randomwalkwithrestartwithjoint.hpp"

bool semi_external;

class SimRank : public RandomWalkwithRestartwithJoint{
private:
	vid_t a, b;
	unsigned R, L;
	std::vector<vid_t> walksfroma; //record the path of walks
	std::vector<vid_t> walksfromb;
    std::string basefilename;
    vid_t cur_window_st;

public:
	void initializeApp( vid_t _a, vid_t _b, unsigned _R, unsigned _L, float tail ){
		a = _a;
		b = _b;
		R = _R;
		L = _L;
		walksfroma.resize(R*L);
		walksfromb.resize(R*L);
		memset(walksfroma.data(), 0xff, walksfroma.size()*sizeof(vid_t));
		memset(walksfromb.data(), 0xff, walksfromb.size()*sizeof(vid_t));
		initializeRW( 2*R, L, tail );
	}

	void startWalksbyApp(WalkManager &walk_manager){
		// R random walk start from a
		logstream(LOG_INFO) << "Start " << R << " walks of length " << L << " from a and b : " << std::endl;
		unsigned nthreads = get_option_int("execthreads", omp_get_max_threads());
		unsigned p = getInterval(a);
		assert( p == getInterval(b) );
		vid_t cur = a - intervals[p].first;
		walk_manager.minstep[p] = 0;
		walk_manager.walknum[p] = 2*R;

		unsigned cap = 2*R/nthreads + 1;
		for( unsigned t = 0; t < nthreads; t++ )
			walk_manager.pwalks[t][p].reserve(cap);

		omp_set_num_threads(nthreads);
		#pragma omp parallel for schedule(static)
			for(unsigned i = 0; i < R; i++){
				unsigned hop = i * L;
				WalkDataType walk = walk_manager.encode(a, cur, hop);
				walk_manager.pwalks[omp_get_thread_num()][p].push_back(walk);
			}
		// R random walk start from b
		cur = b - intervals[p].first;
		#pragma omp parallel for schedule(static)
			for(unsigned i = 0; i < R; i++){
				unsigned hop = i * L;
				WalkDataType walk = walk_manager.encode(b, cur, hop);
				walk_manager.pwalks[omp_get_thread_num()][p].push_back(walk);
			}
		if(!semi_external){
			//write to file
			walk_manager.freshIntervalWalks();
		}
      }

	void updateInfo(WalkManager &walk_manager, WalkDataType walk, vid_t dstId){
		vid_t s = walk_manager.getSourceId(walk);
		unsigned r = walk_manager.getHop(walk);
		if( s == a )
			walksfroma[r] = dstId;
		else if( s == b )
			walksfromb[r] = dstId;
		else{
			logstream(LOG_ERROR) << "Wrong source s : " << s << std::endl;
			assert(0);
		}
		// std::cout << "s , r, dstId = " << s << " " << r << " " << dstId << std::endl;
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

	float computeResult(){
		float simrank = 0;
		for( unsigned i = 0; i < R; i++ )
			for( unsigned j = 0; j < R; j++ )
				for( unsigned l = 0; l < L; l++ ){
					// std::cout << i << " " << j << " " << l << " : " << walksfroma[i*R+l] << " " << walksfromb[j*R+l] << std::endl;
					if( walksfroma[i*L+l] == walksfromb[j*L+l] && walksfroma[i*L+l] != 0xffffffff ){
						simrank += (1.0/(R*R))*pow(0.8, l);
						// std::cout << i << " " << j << " " << l << " " << walksfroma[i*L+l] << " " << simrank << std::endl;
						break;
					}
				}
		return simrank;
	}

};

int main(int argc, const char ** argv) {
    /* Read the command line arguments and the configuration file. */
    set_argc(argc, argv);  
    /* Metrics object for keeping track of performance count_invectorers and other information. Currently required. */
    metrics m("simrank");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file", "../DataSet/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    unsigned nvertices = get_option_int("nvertices", 4847571); // Number of vertices
    long long nedges = get_option_long("nedges", 68993773); // Number of edges
    unsigned nshards = get_option_int("nshards", 0); // Number of intervals
    unsigned a = get_option_int("a", 1); // vertex id of start source
    unsigned b = get_option_int("b", 2); // Number of sources
    unsigned R = get_option_int("R", 1000); // Number of steps
    unsigned L = get_option_int("L", 11); // Number of steps per walk
    float tail = get_option_float("tail", 0); // Ratio of stop long tail
    float prob = get_option_float("prob", 0.2); // prob of chose min step
	semi_external = get_option_int("semi_external", 0);
    
    /* Detect the number of shards or preprocess an input to create them */
    nshards = convert_if_notexists(filename, get_option_string("nshards", "auto"), nvertices, nedges, 2*R, nshards);

    /* Run */
    SimRank program;
    program.initializeApp( a, b, R, L, tail );
    // program.initializeRW( 2*R, L, tail );
    graphwalker_engine engine(filename, nshards, m);
    engine.run(program, prob);

    float simrank = program.computeResult();
    std::cout << "SimRank for " << a << " and " << b << " = " << simrank << std::endl;
    
    /* Report execution metrics */
    metrics_report(m);
    return 0;
}