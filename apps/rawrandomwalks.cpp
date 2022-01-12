#define DYNAMICEDATA 1
#include <string>
#include <fstream>
#include <vector>
#include <cmath>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/randomwalk.hpp"

template<class WalkDataType>
class RawRandomWalks : public RandomWalk<WalkDataType>{

public:

    vid_t N;
    tid_t exec_threads;
    eid_t *used_edges;

    void initializeApp(vid_t _N, wid_t R, hid_t L){
        this->initializeRW(R,L);
        N = _N;
        exec_threads = get_option_int("execthreads", omp_get_max_threads());
        used_edges = new eid_t[exec_threads];
        for(int i=0; i<exec_threads; i++){
            used_edges[i] = 0;
        }

        std::string utilization_filename = "graphwalker_utilization.csv";
        unlink(utilization_filename.c_str()); 
        std::ofstream utilizationfile(utilization_filename.c_str());
        utilizationfile << "runtime \t  walksum  \t nwalks  \t  used_edges[0] \t steps_per_walk  \t  total_edges  \t  utilization  \n" ;
        utilizationfile.close();
    }

    void startWalks(WalkManager<WalkDataType> &walk_manager){
        std::cout << "Random walks:\tStart " << this->R << " walks randomly ..." << std::endl;
        srand((unsigned)time(NULL));
        tid_t exec_threads = get_option_int("execthreads", omp_get_max_threads());
        omp_set_num_threads(exec_threads);
        #pragma omp parallel for schedule(static)
            for (wid_t i = 0; i < this->R; i++){
                vid_t s = rand()%this->N;
                bid_t p = this->getblock(s);
                vid_t cur = s - this->blocks[p];
                WalkDataType walk = WalkDataType(s,cur,0);
                walk_manager.moveWalk(walk,p,omp_get_thread_num(),cur);
            }
        for( bid_t p = 0; p < this->nblocks; p++){
            walk_manager.walknum[p] = walk_manager.dwalknum[p];
            for(tid_t t = 0; t < exec_threads; t++)
                walk_manager.walknum[p] +=  walk_manager.pwalks[t][p].size_w;
            if(walk_manager.walknum[p] )
                walk_manager.minstep[p] = 0;
        }
        walk_manager.walksum = this->R;
    }

    void forwardWalk(WalkDataType walk, wid_t walkid, bid_t walkp, vid_t stv, vid_t env, eid_t *&beg_pos, vid_t *&csr, WalkManager<WalkDataType> &walk_manager ){
        tid_t threadid = omp_get_thread_num();
        WalkDataType nowWalk = walk;
        vid_t sourId = nowWalk.sourceId;
        vid_t dstId = nowWalk.currentId + this->blocks[walkp];
        hid_t hop = nowWalk.hop;
        // logstream(LOG_DEBUG) << sourId << ", from " << dstId << ", " << hop << std::endl;
        unsigned seed = (unsigned)(walkid+dstId+hop+(unsigned)time(NULL));
        while (dstId >= stv && dstId < env && hop < this->L ){
            this->updateInfo(sourId, dstId, threadid, hop);
            vid_t dstIdp = dstId - stv;
            if(stv+1 == env) dstIdp = 0;
            eid_t outd = beg_pos[dstIdp+1] - beg_pos[dstIdp];
            if (outd > 0 && (float)rand_r(&seed)/RAND_MAX > 0.15 ){
                eid_t pos = beg_pos[dstIdp] - beg_pos[0] + ((eid_t)rand_r(&seed))%outd;
                dstId = csr[pos];
            }else{
                dstId = rand_r(&seed) % N;
            }
            hop++;
        }
        if( hop < this->L ){
            bid_t p = this->getblock( dstId );
            if(p >= this->nblocks) return;
            nowWalk = WalkDataType(sourId, dstId - this->blocks[p], hop);
            walk_manager.moveWalk(nowWalk, p, threadid, dstId - this->blocks[p]);
            walk_manager.setMinStep( p, hop );
            walk_manager.ismodified[p] = true;
        }
    }

    void updateInfo(vid_t s, vid_t dstId, tid_t threadid, hid_t hop){
        used_edges[threadid]++;
    }

    
    void compUtilization(eid_t total_edges, wid_t walksum, wid_t nwalks, double runtime){
        for(tid_t i = 1; i < exec_threads; i++){
            used_edges[0] += used_edges[i];
        }

        float utilization = 0;//(float)used_edges[0] / (float)total_edges;
        float steps_per_walk = (float)used_edges[0] / (float)nwalks;
        std::string utilization_filename = "graphwalker_utilization.csv";
        std::ofstream utilizationfile(utilization_filename.c_str(), std::ofstream::app);
        utilizationfile << runtime << "s : \t" << walksum << "\t" << nwalks << "\t" << used_edges[0] << "\t" << steps_per_walk << "\t" << total_edges << "\t" << utilization << "\n" ;
        utilizationfile.close();

        for(tid_t i=0; i<exec_threads; i++){
            used_edges[i] = 0;
        }
    }

};

int main(int argc, const char ** argv){
    set_argc(argc,argv);
    metrics m("randomwalks");
    
    // std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/datasets_for_GraphWalker/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    // std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Friendster/out.friendster-reorder");  // Base filename
    std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Crawl/crawl.txt");  // Base filename
    vid_t N = get_option_int("N", 3563602789); // Number of vertices LJ:4847571 TT:61578415 FS:68349467 YW:1413511394 K30:1073741823 K31:2147483648  CW:3563602789
    wid_t R = get_option_long("R", 1000000); // Number of walks
    hid_t L = get_option_int("L", 10); // Number of steps per walk
    float prob = get_option_float("prob", 0.2); // prob of chose min step
    unsigned long long blocksize_kb = get_option_long("blocksize_kb", 0); // Size of block, represented in KB
    bid_t nmblocks = get_option_int("nmblocks", 0); // number of in-memory blocks
    
    logstream(LOG_DEBUG) << "N R L : " << N << " " << R << " " << L << std::endl;
    // run
    RawRandomWalks<WalkDataType> program;
    program.initializeApp(N,R,L);

    /* Detect the number of shards or preprocess an input to create them */
    if(blocksize_kb == 0){
        blocksize_kb = program.compBlockSize2(R);
    }
    if(nmblocks == 0){
        nmblocks = program.compNmblocks(blocksize_kb);
    }
    bid_t nblocks = convert_if_notexists(filename, blocksize_kb);
    if(nmblocks > nblocks) nmblocks = nblocks;

    logstream(LOG_DEBUG) << "nblocks nmblocks : " << nblocks << " " << nmblocks << std::endl;

    graphwalker_engine<WalkDataType> engine(filename, blocksize_kb, nblocks,nmblocks, m);
    engine.run(program, prob);

    metrics_report(m);
    return 0;
}