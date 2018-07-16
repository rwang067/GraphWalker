#define DYNAMICEDATA 1
#include <string>
#include <fstream>
#include <vector>
#include <cmath>

#include "api/graphwalker_basic_includes.hpp"
#include "walks/randomwalkwithsunk.hpp"


class graphLet : public RandomWalkwithSunk{
    private:
        unsigned N, R, L;
        vid_t a,b;
        bool ans;

    public:
        void initializeApp(unsigned _N, unsigned _R, unsigned _L, float tail, vid_t _a, vid_t _b){
            N = _N;
            R = _R;
            L = _L;
            a = _a;
            b = _b;
            ans = false;
            initializeRW(R, L, tail);
        }

        void startWalksbyApp(WalkManager &walk_manager){
            std::cout << "graphLet:\tStart walks randomly ..." << std::endl;
            srand((unsigned)time(NULL));
            for (unsigned i = 0; i < R; i++){
                int p = getInterval(a);
                vid_t cur = a - intervals[p].first;
                WalkDataType walk = walk_manager.encode(b, cur, 0); // a->b
                walk_manager.walks[p].push_back(walk);
                walk_manager.minstep[p] = 0;
            }
            for (unsigned i = 0; i < R; i++){
                int p = getInterval(b);
                vid_t cur = b - intervals[p].first;
                WalkDataType walk = walk_manager.encode(a, cur, 0); // b->a
                walk_manager.walks[p].push_back(walk);
                walk_manager.minstep[p] = 0;
            }
            walk_manager.freshIntervalWalks();
        }

        void updateInfo(vid_t dstId, unsigned srcId){
            //std::cout<<"updateInfo:\t"<<"dstId="<<dstId<<"\tsrcId="<<srcId<<std::endl;
        }
        void updateInfo(WalkManager &walk_manager, WalkDataType walk, vid_t dstId){
            vid_t finalId = walk_manager.getSourceId(walk);
            if (finalId == dstId){
                //std::cout << "updateInfo:\t" << "finalId=" << finalId << "\tdstId="<<dstId<<std::endl;
                ans = true;
                nsteps = 1;  // halt signal
            }
        }

        bool computeResult(){
            return ans;
        }
};

int main(int argc, const char ** argv){
    set_argc(argc,argv);
    metrics m("randomwalkwithsunk");
    
    std::string filename = get_option_string("file");
    int N = get_option_int("N", 4847576);   // number of vertices
    int R = get_option_int("R", 100000);    // number of walks
    int L = get_option_int("L", 3);         // number of steps per walk
    float tail = get_option_float("tail", 0.05);    // ratio of stop long tail
    float prob = get_option_float("prob", 0.2);     // prob of choosing min step
    int a = get_option_int("a", 1);
    int b = get_option_int("b", 1000000);


    int nshards = convert_if_notexists(filename, get_option_string("nshards", "auto")); //detect the number of shards or preprocess an input to create them.

    // run
    graphLet program;
    program.initializeApp(N,R,L,tail,a,b);
    graphwalker_engine engine(filename, nshards, m);
    engine.run(program, prob);

    std::cout << std::endl;
    std::cout << " === print result ===" << std::endl;
    std::cout << "ans:\t" << program.computeResult() << std::endl;

    metrics_report(m);
    return 0;
}