#include <string>
#include <fstream>
#include <cmath>
#include <fstream>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/randomwalkwithjump.hpp"
#include "util/toplist.hpp"
#include "util/comperror.hpp"

typedef unsigned VertexDataType;

template <typename WalkDataType>
class RandomWalkDomination : public RandomWalkwithJump<WalkDataType>{
public:
    VertexDataType *vertex_value;
    std::string basefilename;
    vid_t cur_window_st;

public:

    static std::string filename_vertex_data(std::string basefilename) {
        std::stringstream ss;
        ss << basefilename;
        ss << "_GraphWalker/" << sizeof(VertexDataType) << "B.vvalue";
        return ss.str();
    }

    void initializeApp( vid_t _N, wid_t _R, hid_t _L, std::string _basefilename ){
        this->initializeRW( _N, _R, _L);
        basefilename = _basefilename;
        vertex_value = new VertexDataType[this->N];
        for(vid_t i = 0; i < this->N; i++){
            vertex_value[i] = 0;
        }
        // initialVertexValue<VertexDataType>(N, basefilename);
    }

    void startWalksbyApp( WalkManager<WalkDataType> &walk_manager  ){
        //muti threads to start walks
        logstream(LOG_INFO) << "Start walks ! Total walk number = " << this->R * this->N << std::endl;
        tid_t nthreads = get_option_int("execthreads", omp_get_max_threads());
        omp_set_num_threads(nthreads);
        #pragma omp parallel for schedule(static)
            for( bid_t p = 0; p < this->nblocks; p++ ){
                walk_manager.minstep[p] = 0;
                walk_manager.walknum[p] = (this->blocks[p+1]-this->blocks[p])*this->R;
                
                for( vid_t v = this->blocks[p]; v < this->blocks[p+1]; v++ ){
                    vid_t s = v;
                    vid_t cur = s - this->blocks[p];
                    WalkDataType walk = WalkDataType(s, cur, 0);
                    for( wid_t j = 0; j < this->R; j++ ){
                        walk_manager.moveWalk(walk,p,omp_get_thread_num(),cur);
                    }
                }
            }
        walk_manager.walksum = this->R * this->N;
    }

	void updateInfo(vid_t s, vid_t dstId, tid_t threadid, hid_t hop){
        assert(dstId < this->N);
        vertex_value[dstId]++; // #pragma omp critical
    }

    
    void compUtilization(eid_t total_edges, wid_t walksum, wid_t nwalks, double runtime){
        std::string utilization_filename = "graphwalker_utilization.csv";
        std::ofstream utilizationfile(utilization_filename.c_str(), std::ofstream::app);
        utilizationfile << runtime << "s : \t" << walksum << "\t" << nwalks << "\t" << total_edges  << "\n" ;
        utilizationfile.close();
    }

};


int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line
     arguments and the configuration file. */
    set_argc(argc, argv);
    
    /* Metrics object for keeping track of performance count_invectorers
     and other information. Currently required. */
    metrics m("RWDomination");
    
    /* Basic arguments for application */
    // std::string filename = get_option_string("file", "../dataset/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    // std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Twitter/twitter_rv.net");  // Base filename
    // std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Yahoo/yahoo-webmap.txt");  // Base filename
    // std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Kron30/kron30_32-sorted.txt");  // Base filename
    std::string filename = get_option_string("file", "../../data/raid0_defghij_ssd/datasets_for_GraphWalker/Crawl/crawl.txt");  // Base filename
    unsigned N = get_option_int("N", 3563602789); // Number of vertices LJ:4847571 TT:61578415 FS:68349467 YW:1413511394 K30:1073741823 K31:2147483648  CW:3563602789
    unsigned R = get_option_int("R", 1); // Number of steps
    unsigned L = get_option_int("L", 6); // Number of steps per walk
    float prob = get_option_float("prob", 0.2); // prob of chose min step
    unsigned long long blocksize_kb = get_option_long("blocksize_kb", 0); // Size of block, represented in KB
    bid_t nmblocks = get_option_int("nmblocks", 0); // number of in-memory blocks
    
    /* Run */
    RandomWalkDomination<WalkDataType> program;
    program.initializeApp( N, R, L, filename );

    if(blocksize_kb == 0)
        blocksize_kb = program.compBlockSize(N*R);
    /* Detect the number of shards or preprocess an input to create them */
    bid_t nblocks = convert_if_notexists(filename, blocksize_kb);
    if(nmblocks == 0) nmblocks = program.compNmblocks(blocksize_kb);
    if(nmblocks > nblocks) nmblocks = nblocks;

    graphwalker_engine<WalkDataType> engine(filename, blocksize_kb, nblocks,nmblocks, m);
    engine.run(program, prob);

    /* Report execution metrics */
    metrics_report(m);
    return 0;
}