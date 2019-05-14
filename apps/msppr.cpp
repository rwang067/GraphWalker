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
    vid_t firstsource, numsources;
    wid_t walkspersource;
    hid_t maxwalklength;
    DiscreteDistribution *visitfrequencies;

public:

    void initializeApp(vid_t _firstsource, vid_t _numsources, wid_t _walkspersource, hid_t _maxwalklength){
        firstsource = _firstsource;
        numsources = _numsources;
        walkspersource = _walkspersource;
        maxwalklength = _maxwalklength;
        initializeRW(numsources*walkspersource, maxwalklength);
        visitfrequencies = new DiscreteDistribution[numsources];
        logstream(LOG_INFO) << "Successfully allocate visitfrequencies memory for each each source, with numsources = " << numsources << std::endl;
    }

    void startWalksbyApp(WalkManager &walk_manager, std::string base_filename){
        logstream(LOG_INFO) << "Start walks ! Total walk number = " << numsources*walkspersource << std::endl;
        bid_t p = getblock(firstsource);
        vid_t sts = firstsource, ens = blocks[p+1], nums;
        vid_t count = numsources;
        walk_manager.walksum = 0;
        while(count > 0){
            if(ens > firstsource+numsources) 
                ens = firstsource+numsources;
            logstream(LOG_INFO) << "Start walks of sources : [" << sts << ", " << ens << ")"<< std::endl;
            nums = ens - sts;
            count -= nums;
            walk_manager.minstep[p] = 0;
            walk_manager.walknum[p] = nums*walkspersource;
            logstream(LOG_INFO) << "walk_manager.walknum[p] = " << walk_manager.walknum[p] << std::endl;
            WalkDataType* curwalks = (WalkDataType*)malloc(walk_manager.walknum[p]*sizeof(WalkDataType));
            logstream(LOG_INFO) << "walk_manager.walknum[p] = " << walk_manager.walknum[p] << std::endl;
            #pragma omp parallel for schedule(static)
                for(vid_t s = 0; s < nums; s++){
                    vid_t cur = s + sts - blocks[p];
                    WalkDataType walk = walk_manager.encode(s + sts - firstsource, cur, 0);
                    for( wid_t j = 0; j < walkspersource; j++ ){
                        curwalks[s*walkspersource+j] = walk;
                    }
                }
            std::string walksfile = walksname( base_filename, p );
            int f = open(walksfile.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, S_IROTH | S_IWOTH | S_IWUSR | S_IRUSR);
            pwritea( f, &curwalks[0], walk_manager.walknum[p]*sizeof(WalkDataType) );
            close(f);
            free(curwalks);
            curwalks = NULL;
            walk_manager.dwalknum[p] = walk_manager.walknum[p];
            walk_manager.walksum += walk_manager.walknum[p];
            p++;
            sts = ens;
            ens = blocks[p+1];
        }
    }

    void updateInfo(vid_t s, vid_t dstId, tid_t threadid, hid_t hop){
        // logstream(LOG_INFO) << "updateInfo in msppr." << std::endl;
        visitfrequencies[s].add(dstId);
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
    std::string filename = get_option_string("file", "../../raid0_mnop/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    vid_t firstsource = get_option_int("firstsource", 0); // vertex id of start source
    vid_t numsources = get_option_int("numsources", 10); // Number of sources
    wid_t walkspersource = get_option_int("walkspersource", 2000); // Number of steps
    hid_t maxwalklength = get_option_int("maxwalklength", 10); // Number of steps per walk
    float prob = get_option_float("prob", 0.2); // prob of chose min step
    unsigned long long blocksize_kb = get_option_long("blocksize_kb", 32768); // Size of block, represented in KB
    bid_t nmblocks = get_option_long("nmblocks", 10); // number of in-memory blocks
    
    /* Detect the number of shards or preprocess an input to create them */
    bid_t nblocks = convert_if_notexists(filename, blocksize_kb);

    // char cmd[256];
	// // sprintf(cmd,"%s","iostat -x 1 -k > iostat_randomwalks.log&");
	// sprintf(cmd,"%s","top -b -d 5 -u wang > top_msppr.log&");
	// std::cout<<cmd<<"\n";
    // system((const char *)cmd);

    /* Run */
    MultiSourcePersonalizedPageRank program;
    program.initializeApp(firstsource, numsources, walkspersource, maxwalklength);
    graphwalker_engine engine(filename, blocksize_kb, nblocks,nmblocks, m);
    engine.run(program, prob);

    program.visitfrequencies[0].getTop(20);

    system("killall top");
    /* Report execution metrics */
    metrics_report(m);
    return 0;
}