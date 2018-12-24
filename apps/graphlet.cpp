#define DYNAMICEDATA 1
#include <string>
#include <fstream>
#include <vector>
#include <cmath>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/randomwalkwithsunk.hpp"

bool semi_external;


class graphLet : public RandomWalkwithSunk{
    private:
        unsigned N, R, L;
        unsigned *cnt_all;
        unsigned *cnt_ok;

    public:
        void initializeApp(unsigned _N, unsigned _R, unsigned _L, float tail){
            N = _N;
            R = _R;
            L = _L;
            cnt_all = 0;
            cnt_ok = 0;
            initializeRW(R, L, tail);
        }

        void startWalksbyApp(WalkManager &walk_manager){
            std::cout << "graphLet:\tStart walks randomly ..." << std::endl;
            srand((unsigned)time(NULL));
            unsigned nthreads = get_option_int("execthreads", omp_get_max_threads());
            omp_set_num_threads(nthreads);
            #pragma omp parallel for schedule(static)
                for (unsigned i = 0; i < nwalks; i++){
                    vid_t s = rand()%N;
                    int p = getInterval(s);
                    vid_t cur = s - intervals[p].first;
                    //std::cout << "startWalksbyApp:\t" << "source" << i <<"=" << s << "\tp=" << p << "\tcur=" << cur << std::endl;
                    WalkDataType walk = walk_manager.encode(s,cur,0);
                    walk_manager.pwalks[omp_get_thread_num()][p].push_back(walk);
                    walk_manager.minstep[p] = 0;
                    walk_manager.walknum[p]++;
                }
            cnt_all = new unsigned [nthreads];
            cnt_ok = new unsigned [nthreads];
            for(unsigned i = 0; i < nthreads; i++ ){
                cnt_all[i] = 0;
                cnt_ok[i] = 0;
            }
            if(!semi_external){
                walk_manager.freshIntervalWalks();
            }
        }

        void updateInfo(WalkManager &walk_manager, WalkDataType walk, vid_t dstId){
		    unsigned hop = walk_manager.getHop(walk);
            if(hop < L-1) return;
		    vid_t srcId = walk_manager.getSourceId(walk);
            unsigned threadid = omp_get_thread_num();
            cnt_all[threadid]++;
            if (dstId == srcId){
                cnt_ok[threadid]++;
            }
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
            unsigned nthreads = get_option_int("execthreads", omp_get_max_threads());
            for(unsigned i = 1; i < nthreads; i++ ){
                cnt_all[0] += cnt_all[i];
                cnt_ok[0] += cnt_ok[i];
            }
            float triangle_ratio = (float) cnt_ok[0] / (float) cnt_all[0];
            return triangle_ratio;
        }
        unsigned getCntAll(){
            return cnt_all[0];
        }
        unsigned getCntOK(){
            return cnt_ok[0];
        }
};

int main(int argc, const char ** argv){
    set_argc(argc,argv);
    metrics m("graphlet");
    
    std::string filename = get_option_string("file", "../DataSet/LiveJournal/soc-LiveJournal1.txt");  // Base filename
    unsigned nvertices = get_option_int("nvertices", 4847571); // Number of vertices
    unsigned R = get_option_int("R", 10000); // Number of steps
    unsigned L = get_option_int("L", 4); // Number of steps per walk
    float tail = get_option_float("tail", 0); // Ratio of stop long tail
    float prob = get_option_float("prob", 0.2); // prob of chose min step
    semi_external = get_option_int("semi_external", 0);
    long long shardsize = get_option_long("shardsize", 0); // Size of shard, represented in KB

    /* Detect the number of shards or preprocess an input to create them */
    unsigned nshards = convert_if_notexists(filename, shardsize);
    
    // run
    graphLet program;
    program.initializeApp(nvertices,R,L,tail);
    graphwalker_engine engine(filename, shardsize, nshards, m);
    engine.run(program, prob);

    float triangle_ratio = program.computeResult();
    std::cout << "The ratio of triangle:\t" << triangle_ratio << std::endl;
    std::cout << "cnt_all:\t" << program.getCntAll() << std::endl;
    std::cout << "cnt_ok:\t" << program.getCntOK() << std::endl;

    metrics_report(m);
    return 0;
}