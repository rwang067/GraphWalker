
#define KEEPWALKSINDISK

#include <string>
#include <fstream>
#include <cmath>
#include <fstream>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/randomwalkwithstop.hpp"
#include "walks/discretedistribution.hpp"
#include "util/toplist.hpp"
#include "util/comperror.hpp"

class MultiSourcePersonalizedPageRank : public RandomWalkwithStop{
public:
    vid_t firstsource;
    wid_t numsources, walkspersource; 
    hid_t maxwalklength;
    DiscreteDistribution *visitfrequencies;

public:

    void initializeApp(vid_t _firstsource, wid_t _numsources, wid_t _walkspersource, hid_t _maxwalklength){
        firstsource = _firstsource;
        numsources = _numsources;
        walkspersource = _walkspersource;
        maxwalklength = _maxwalklength;
        initializeRW(numsources*walkspersource, maxwalklength);
        #ifdef KEEPWALKSINDISK
            visitfrequencies = new DiscreteDistribution[numsources];
        #else
            visitfrequencies = new DiscreteDistribution[numsources];
        #endif
        logstream(LOG_INFO) << "Successfully allocate visitfrequencies memory for each each source, with numsources = " << numsources << std::endl;
    }

    void startWalksbyApp( WalkManager &walk_manager  ){
        logstream(LOG_INFO) << "Start walks ! Total walk number = " << numsources*walkspersource << std::endl;
        unsigned nthreads = get_option_int("execthreads", omp_get_max_threads());
        while(numsources > 0){
            sid_t p = getInterval(firstsource);

            vid_t lastsource = firstsource + numsources - 1;
            if( lastsource >= intervals[p+1] ){
                numsources = lastsource - intervals[p+1] + 1;
                lastsource = intervals[p+1] - 1;
            }else{
                numsources = 0;
            }

            wid_t walknum = (lastsource-firstsource+1)*walkspersource;
            walk_manager.pwalks[p] = (WalkDataType*)malloc(sizeof(WalkDataType) * walknum);

            logstream(LOG_INFO) << "Start walks from sources [ " << firstsource << " , " << lastsource << " ]" << std::endl;
            omp_set_num_threads(nthreads);
            for(vid_t s = firstsource; s <= lastsource; s++){
                vid_t cur = s - intervals[p];
                WalkDataType walk = walk_manager.encode(s-firstsource, cur, 0);
                for( wid_t j = 0; j < walkspersource; j++ ){
                    walk_manager.pwalks[p][ walk_manager.pnwalks[p]++ ] = walk;
                }
                // logstream(LOG_DEBUG) << "walk_manager.pnwalks[p] " << walk_manager.pnwalks[p] << std::endl;
                if(s%50000==0) logstream(LOG_DEBUG) << s << std::endl;
            }
        }
        #ifdef KEEPWALKSINDISK
            walk_manager.writeWalkPools();
        #endif
    }

    void updateInfo(vid_t s, vid_t dstId, unsigned threadid, hid_t hop){
        #ifdef KEEPWALKSINDISK
            visitfrequencies[s].add(dstId);
        #else
            visitfrequencies[s].add(dstId);
        #endif
    }

    /**
     * Called before an execution interval is started.
     */
    void before_exec_interval(sid_t exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
    }
    
    /**
     * Called after an execution interval has finished.
     */
    void after_exec_interval(sid_t exec_interval, vid_t window_st, vid_t window_en, WalkManager &walk_manager) {
    }
};


int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line
     arguments and the configuration file. */
    set_argc(argc, argv);
    
    /* Metrics object for keeping track of performance count_invectorers
     and other information. Currently required. */
    metrics m("multi-source-personalizedpagerank");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file", "/data/rwang067/Wiki/wikipedia_sorted.data");  // Base filename
    vid_t firstsource = get_option_int("firstsource", 1); // vertex id of start source
    wid_t numsources = (wid_t)get_option_int("numsources", 100000); // Number of sources
    wid_t walkspersource = (wid_t)get_option_int("walkspersource", 2000); // Number of steps
    hid_t maxwalklength = (hid_t)get_option_int("maxwalklength", 10); // Number of steps per walk
    float prob = get_option_float("prob", 0.2); // prob of chose min step
    long long shardsize = get_option_long("shardsize", 1048576); // Size of shard, represented in KB
    
    /* Detect the number of shards or preprocess an input to create them */
    sid_t nshards = convert_if_notexists(filename, shardsize);

    /* Run */
    MultiSourcePersonalizedPageRank program;
    program.initializeApp(firstsource, numsources, walkspersource, maxwalklength);
    graphwalker_engine engine(filename, shardsize, nshards, m);
    engine.run(program, prob);

    program.visitfrequencies[0].getTop(20);

    /* Report execution metrics */
    metrics_report(m);
    return 0;
}