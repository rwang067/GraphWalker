
#define DYNAMICEDATA 1

#include <string>
#include <fstream>
#include <vector>
#include <cmath>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/simplerandomwalk.hpp"

class AvgDegree : public SimpleRandomWalk{
private:
    unsigned N, R, L;
    unsigned *deg;
    unsigned r;

public:
    void initializeApp( unsigned _N, unsigned _R, unsigned _L, float tail ){
        N = _N;
        R = _R;
        L = _L;
        deg = (unsigned*)malloc(R*sizeof(unsigned));
        memset(deg, 0, R*sizeof(unsigned));
        initializeRW( R, L, tail );
        r = 0;
    }

    void startWalksbyApp(WalkManager &walk_manager){
        // R random walk start from a
        std::cout << "Start walks randomly : " << std::endl;
        srand((unsigned)time(NULL));
        for( int i = 0; i < nwalks; i++ ){
            vid_t s = rand() % N;
            int p = getInterval(s);
            vid_t cur = s - intervals[p].first;
            WalkDataType walk = walk_manager.encode(s, cur, 0);
            walk_manager.walks[p].push_back(walk);
            walk_manager.minstep[p] = 0;
        }
        walk_manager.freshIntervalWalks();
      }

    void updateInfo(vid_t v,  unsigned d){
        // std::cout <<"v d : " << v << " " << d << std::endl;
        if( d > 0 )
            deg[r++] = d;
    }

    float computeResult(){
        std::cout <<" r : " << r << std::endl;
        float avgdeg = 0;
        for( unsigned i = 0; i < r; i++ )
            avgdeg += 1.0/deg[i];
        avgdeg = r/avgdeg;
        return avgdeg;
    }

};

int main(int argc, const char ** argv) {
    /* Read the command line arguments and the configuration file. */
    set_argc(argc, argv);  
    /* Metrics object for keeping track of performance count_invectorers and other information. Currently required. */
    metrics m("simrank");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file", "/home/wang/Documents/graph processing system/dataset/LiveJournal1/soc-LiveJournal1.txt");  // Base filename
    int N = get_option_int("N", 4847576); // Number of sources
    int R = get_option_int("R", 1000); // Number of steps
    int L = get_option_int("L", 1000); // Number of steps per walk
    float tail = get_option_float("tail", 0.05); // Ratio of stop long tail
    float prob = get_option_float("prob", 0.2); // prob of chose min step
    
    /* Detect the number of shards or preprocess an input to create them */
    int nshards = convert_if_notexists(filename, get_option_string("nshards", "auto"));

    /* Run */
    AvgDegree program;
    program.initializeApp( N, R, L, tail );
    // program.initializeRW( 2*R, L, tail );
    graphwalker_engine engine(filename, nshards, m);
    engine.run(program, prob);

    float avgdeg = program.computeResult();
    std::cout << "Average degree : " << avgdeg << std::endl;

    float err = std::abs(avgdeg-13.5988)/13.5988;
    std::ofstream errfile;
    errfile.open("/home/wang/Documents/graph processing system/dataset/LiveJournal_undirected/avgdegree.error", std::ofstream::app);
    errfile << err << "\n" ;
    errfile.close();
    
    /* Report execution metrics */
    metrics_report(m);
    return 0;
}