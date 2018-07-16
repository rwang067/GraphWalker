
#define DYNAMICEDATA 1

#include <string>
#include <fstream>
#include <vector>
#include <cmath>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/simplerandomwalk.hpp"

class SimRank : public SimpleRandomWalk{
private:
	vid_t a, b;
	unsigned R, L;
	std::vector<vid_t> walksfroma;
	std::vector<vid_t> walksfromb;

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
		std::cout << "Start walks from a and b : " << std::endl;
		int p = getInterval(a);
            vid_t cur = a - intervals[p].first;
          	walk_manager.minstep[p] = 0;
          	for(unsigned i = 0; i < R; i++){
          		int hop = i * L;
          		WalkDataType walk = walk_manager.encode(a, cur, hop);
          		walk_manager.walks[p].push_back(walk);
          	}
		// R random walk start from b
		p = getInterval(b);
            cur = b - intervals[p].first;
          	walk_manager.minstep[p] = 0;
          	for(unsigned i = 0; i < R; i++){
          		int hop = i * L;
          		WalkDataType walk = walk_manager.encode(b, cur, hop);
          		walk_manager.walks[p].push_back(walk);
          	}
          	walk_manager.freshIntervalWalks();
      }

	void updateInfo(WalkManager &walk_manager, WalkDataType walk, vid_t dstId){
		vid_t s = walk_manager.getSourceId(walk);
		int r = walk_manager.getHop(walk);
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

	float computeResult(){
		float simrank = 0;
		for( unsigned i = 0; i < R; i++ )
			for( unsigned j = 0; j < R; j++ )
				for( unsigned l = 0; l < L; l++ ){
					// std::cout << i << " " << j << " " << l << " : " << walksfroma[i*R+l] << " " << walksfromb[j*R+l] << std::endl;
					if( walksfroma[i*L+l] == walksfromb[j*L+l] && walksfroma[i*L+l] != 0xffffffff ){
						simrank += (1.0/(R*R))*pow(0.8, l);
						std::cout << i << " " << j << " " << l << " " << walksfroma[i*L+l] << " " << simrank << std::endl;
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
    std::string filename = get_option_string("file", "/home/wang/Documents/graph processing system/dataset/LiveJournal1/soc-LiveJournal1.txt");  // Base filename
    int a = get_option_int("a", 0); // vertex id of start source
    int b = get_option_int("b", 1); // Number of sources
    int R = get_option_int("R", 1600); // Number of steps
    int L = get_option_int("L", 10); // Number of steps per walk
    float tail = get_option_float("tail", 0.05); // Ratio of stop long tail
    float prob = get_option_float("prob", 0.2); // prob of chose min step
    
    /* Detect the number of shards or preprocess an input to create them */
    int nshards = convert_if_notexists(filename, get_option_string("nshards", "auto"));

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