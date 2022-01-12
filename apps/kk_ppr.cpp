#include <string>
#include <fstream>
#include <cmath>
#include <fstream>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/randomwalk.hpp"

template<class WalkDataType>
class MultiSourcePersonalizedPageRank : public RandomWalk<WalkDataType>{
public:
    vid_t firstsource, numsources;
    wid_t walkspersource;
    hid_t maxwalklength;

public:

    void initializeApp(vid_t _firstsource, vid_t _numsources, wid_t _walkspersource, hid_t _maxwalklength){
        firstsource = _firstsource;
        numsources = _numsources;
        walkspersource = _walkspersource;
        maxwalklength = _maxwalklength;
        this->initializeRW(numsources*walkspersource, maxwalklength);
    }

    void startWalks(WalkManager<WalkDataType> &walk_manager){
        logstream(LOG_INFO) << "Start walks ! Total walk number = " << numsources*walkspersource << std::endl;
        bid_t p = this->getblock(firstsource);
        vid_t sts = firstsource, ens = this->blocks[p+1], nums;
        vid_t count = numsources;
        walk_manager.walksum = 0;
        while(count > 0){
            if(ens > firstsource+numsources) 
                ens = firstsource+numsources;
            logstream(LOG_INFO) << "Start walks of sources : [" << sts << ", " << ens << ") of block " << p << ", blocks[p+1] = " << this->blocks[p+1] << std::endl;
            nums = ens - sts;
            count -= nums;
            walk_manager.minstep[p] = 0;
            walk_manager.walknum[p] = nums*walkspersource;
            #pragma omp parallel for schedule(static)
            for(vid_t s = 0; s < nums; s++){
                vid_t cur = s + sts - this->blocks[p];
                WalkDataType walk = WalkDataType(s + sts - firstsource, cur, 0);
                for( wid_t j = 0; j < walkspersource; j++ ){
                    walk_manager.moveWalk(walk,p,omp_get_thread_num(),cur);
                }
            }
            walk_manager.walksum += walk_manager.walknum[p];
            p++;
            sts = ens;
            ens = this->blocks[p+1];
        }
    }

    void forwardWalk(WalkDataType walk, wid_t walkid, bid_t walkp, vid_t stv, vid_t env, eid_t *&beg_pos, vid_t *&csr, WalkManager<WalkDataType> &walk_manager ){
        tid_t threadid = omp_get_thread_num();
        WalkDataType nowWalk = walk;
        vid_t sourId = nowWalk.sourceId;
        vid_t dstId = nowWalk.currentId + this->blocks[walkp];
        hid_t hop = nowWalk.hop;
        unsigned seed = (unsigned)(walkid+dstId+hop+(unsigned)time(NULL));
        while (dstId >= stv && dstId < env && hop < this->L ){
            vid_t dstIdp = dstId - stv;
            if(stv+1 == env) assert(dstIdp == 0);
            eid_t outd = beg_pos[dstIdp+1] - beg_pos[dstIdp];
            if (outd > 0 && (float)rand_r(&seed)/RAND_MAX > 0.15 ){
                eid_t pos = beg_pos[dstIdp] - beg_pos[0] + ((eid_t)rand_r(&seed))%outd;
                dstId = csr[pos];
            }else{
                return; // stop
            }
            hop++;
        }
        if( hop < this->L ){
            bid_t p = this->getblock(dstId);
            if(p >= this->nblocks) return;
            nowWalk = WalkDataType(sourId, dstId - this->blocks[p], hop);
            walk_manager.moveWalk(nowWalk, p, threadid, dstId - this->blocks[p]);
            walk_manager.setMinStep( p, hop );
            walk_manager.ismodified[p] = true;
        }
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
    // std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/datasets_for_GraphWalker/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/twitter_rv.net");  // Base filename
    vid_t firstsource = get_option_int("firstsource", 12); // vertex id of start source
    vid_t numsources = get_option_int("numsources", 100); // Number of sources, 1000, 1000000
    wid_t walkspersource = get_option_long("walkspersource", 2000); // Number of steps
    hid_t maxwalklength = get_option_int("maxwalklength", 10); // Number of steps per walk, max 16384
    float prob = get_option_float("prob", 0.2); // prob of chose min step

    unsigned long long blocksize_kb = get_option_long("blocksize_kb", 2048); // Size of block, represented in KB
    bid_t nmblocks = get_option_int("nmblocks", 0); // number of in-memory blocks
    wid_t fg_threshold =  get_option_int("fg_threshold", 100000); // threshold of #walks for fine-grained graph loading

    /* Run */
    MultiSourcePersonalizedPageRank<WalkDataType> program;
    program.initializeApp(firstsource, numsources, walkspersource, maxwalklength);

    m.start_time("prepare_graph");
    if(blocksize_kb == 0)
        blocksize_kb = program.compBlockSize(numsources*walkspersource);
    /* Detect the number of shards or preprocess an input to create them */
    bid_t nblocks = convert_if_notexists(filename, blocksize_kb);
    if(nmblocks == 0) nmblocks = program.compNmblocks(blocksize_kb);
    if(nmblocks > nblocks) nmblocks = nblocks;
    m.stop_time("prepare_graph");

    logstream(LOG_INFO) << "parameters, nblocks: " << 
                           nblocks << ", nmblocks: " << 
                           nmblocks << ", blocksize: " << blocksize_kb << " KB" << std::endl;

    graphwalker_engine<WalkDataType> engine(filename, blocksize_kb,nblocks,nmblocks, m);
    engine.run(program, prob, fg_threshold);

    /* Report execution metrics */
    metrics_report(m);
    return 0;
}