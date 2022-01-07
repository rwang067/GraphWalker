#define DYNAMICEDATA 1
#include <string>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>

#include "api/graphwalker_basic_includes.hpp"
#include "api/datatype.hpp"

class Node2VecWalkDataType : public WalkDataType{
public:
    vid_t last_step;
    vid_t* last_adjlist;
    eid_t last_degree;
    Node2VecWalkDataType(vid_t _sourceId, vid_t _currentId, hid_t _hop, vid_t _last_step, vid_t* _last_adjlist, eid_t _last_degree):
        WalkDataType(_sourceId, _currentId, _hop), 
        last_step(_last_step), last_adjlist(_last_adjlist), last_degree(_last_degree){}
};

class StdRandNumGenerator
{
    std::random_device *rd;
    std::mt19937 *mt;
public:
    StdRandNumGenerator()
    {
        rd = new std::random_device();
        mt = new std::mt19937((*rd)());
    }
    ~StdRandNumGenerator()
    {
        delete mt;
        delete rd;
    }
    vid_t gen(vid_t upper_bound)
    {
        std::uniform_int_distribution<vid_t> dis(0, upper_bound - 1);
        return dis(*mt);
    }
    float gen_float(float upper_bound)
    {
        std::uniform_real_distribution<float> dis(0.0, upper_bound);
        return dis(*mt);
    }
};

template<class WalkDataType>
class Node2Vec : public RandomWalk<WalkDataType> {
public:
    vid_t N;
    float p;
    float q;
    float upperbound;
    float lowerbound;
    StdRandNumGenerator* randgen;

    void initializeApp(vid_t _N, hid_t L, float _p, float _q){
        N = _N;
        p = _p;
        q = _q;
        this->initializeRW(N,L);
        upperbound = std::max(1.0, 1.0 / q);
        lowerbound = std::min(1.0 / p, std::min(1.0, 1.0 / q));
        randgen = new StdRandNumGenerator[omp_get_max_threads()];
    }

    void startWalksbyApp(WalkManager<WalkDataType> &walk_manager){
        std::cout << "Random walks:\tStart " << 1 << " walk from each vertex ..." << std::endl;
        srand((unsigned)time(NULL));
        tid_t exec_threads = get_option_int("execthreads", omp_get_max_threads());
        omp_set_num_threads(exec_threads);
        #pragma omp parallel for schedule(static)
            for (wid_t i = 0; i < N; i++){
                vid_t s = i;
                bid_t p = this->getblock(s);
                vid_t cur = s - this->blocks[p];
                WalkDataType walk = WalkDataType(s, cur, 0, s, 0, 0);
                walk_manager.moveWalk(walk,p,omp_get_thread_num(),cur);
            }
        for( bid_t p = 0; p < this->nblocks; p++){
            walk_manager.walknum[p] = walk_manager.dwalknum[p];
            for(tid_t t = 0; t < exec_threads; t++)
                walk_manager.walknum[p] +=  walk_manager.pwalks[t][p].size_w;
            if(walk_manager.walknum[p] )
                walk_manager.minstep[p] = 0;
        }
        walk_manager.walksum = N;
    }

    void updateByWalk(WalkDataType walk, wid_t walkid, bid_t walkp, vid_t stv, vid_t env, eid_t *&beg_pos, vid_t *&csr, WalkManager<WalkDataType> &walk_manager ){ 
        tid_t threadid = omp_get_thread_num();
        StdRandNumGenerator* gen = &randgen[threadid];
        WalkDataType nowWalk = walk;
        vid_t sourId = nowWalk.sourceId;
        vid_t last_step = nowWalk.last_step;
        vid_t* last_adjlist = nowWalk.last_adjlist;
        eid_t last_degree = nowWalk.last_degree;
        vid_t curId = nowWalk.currentId + this->blocks[walkp];
        hid_t hop = nowWalk.hop;
        unsigned seed = (unsigned)(walkid+curId+hop+(unsigned)time(NULL));
        while (curId >= stv && curId < env && hop < this->L ){
            updateInfo(sourId, curId, threadid, hop);
            vid_t curIdp = curId - stv;
            if(stv+1 == env) curIdp = 0;
            eid_t outd = beg_pos[curIdp+1] - beg_pos[curIdp];
            if (outd > 0){
                // vid_t* cur_adjlist = (vid_t*)malloc(sizeof(vid_t)*outd);
                // memcpy(cur_adjlist, csr + (beg_pos[curIdp] - beg_pos[0]), sizeof(vid_t)*outd);
                vid_t* cur_adjlist = csr + (beg_pos[curIdp] - beg_pos[0]);
                bool accept = false; 
                vid_t nextId = 0;
                while(!accept){
                    eid_t pos = rand_r(&seed)%outd;
                    nextId = cur_adjlist[pos];
                    float acc_prob = gen->gen_float(upperbound);//rand_r(&seed)%upperbound;
                    if(hop > 0 && nextId == last_step){ // d = 0
                        if(acc_prob <= 1/p){
                            accept = true;
                        }
                    } else if(hop > 0 && std::binary_search(last_adjlist, last_adjlist + last_degree, nextId)){ // d = 1, adjacent
                    // } else if(hop > 0 && std::find(last_adjlist, last_adjlist + last_degree, nextId)){ // d = 1, adjacent
                        if(acc_prob <= 1){
                            accept = true;
                        }
                    } else {
                        if(acc_prob <= 1/q){
                            accept = true;
                        }
                    }
                }
                // logstream(LOG_DEBUG) << "hop_" << hop  << ": " << curId << " -> " << nextId << std::endl;
                // if(last_adjlist != 0) free(last_adjlist);
                last_adjlist = cur_adjlist;
                last_degree = outd;
                last_step = curId;
                curId = nextId;
                hop++;
            }else{
                return;
            }
        }
        if(hop == 0){
            logstream(LOG_INFO) << walk.sourceId << " " << walk.currentId << " " << walk.hop << std::endl;
            logstream(LOG_FATAL) << curId << " not in [" << stv << ", " << env << "), hop = " << hop << std::endl;
            return;
        }
        if( hop < this->L ){
            bid_t p = this->getblock( curId );
            if(p >= this->nblocks) return;
            vid_t* last_adjlist1 = (vid_t*)malloc(sizeof(vid_t)*last_degree);
            memcpy(last_adjlist1, last_adjlist, sizeof(vid_t)*last_degree);
            nowWalk = WalkDataType(sourId, curId - this->blocks[p], hop, last_step, last_adjlist1, last_degree);
            walk_manager.moveWalk(nowWalk, p, threadid, curId - this->blocks[p]);
            walk_manager.setMinStep( p, hop );
            walk_manager.ismodified[p] = true;
        }
    }

    void updateInfo(vid_t s, vid_t dstId, tid_t threadid, hid_t hop){
    }
};

int main(int argc, const char ** argv){
    set_argc(argc,argv);
    metrics m("randomwalks");
    
    // std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/datasets_for_GraphWalker/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Friendster/out.friendster-reorder");  // Base filename
    // std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Crawl/crawl.txt");  // Base filename
    vid_t N = get_option_int("N", 68349467); // Number of vertices LJ:4847571 TT:61578415 FS:68349467 YW:1413511394 K30:1073741823 K31:2147483648  CW:3563602789
    hid_t L = get_option_int("L", 10); // Number of steps per walk
    float p = get_option_float("p", 0.5); // prob of chose min step
    float q = get_option_float("q", 2.0); // prob of chose min step

    float prob = get_option_float("prob", 0.2); // prob of chose min step
    unsigned long long blocksize_kb = get_option_long("blocksize_kb", 0); // Size of block, represented in KB
    bid_t nmblocks = get_option_int("nmblocks", 0); // number of in-memory blocks
    
    logstream(LOG_DEBUG) << "N L p q: " << N << " " << L << " " << p << " " << q << std::endl;
    // run
    Node2Vec<Node2VecWalkDataType> program;
    program.initializeApp(N,L,p,q);

    /* Detect the number of shards or preprocess an input to create them */
    if(blocksize_kb == 0){
        blocksize_kb = program.compBlockSize2(N);
    }
    if(nmblocks == 0){
        nmblocks = program.compNmblocks(blocksize_kb);
    }
    bid_t nblocks = convert_if_notexists(filename, blocksize_kb);
    if(nmblocks > nblocks) nmblocks = nblocks;

    logstream(LOG_DEBUG) << "nblocks nmblocks : " << nblocks << " " << nmblocks << std::endl;

    graphwalker_engine<Node2VecWalkDataType> engine(filename, blocksize_kb, nblocks,nmblocks, m);
    engine.run(program, prob);

    metrics_report(m);
    return 0;
}