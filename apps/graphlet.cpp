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
        unsigned cnt_all;
        unsigned cnt_ok;

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
            for (int i = 0; i < nwalks; i++){
                vid_t s = rand()%N;
                int p = getInterval(s);
                vid_t cur = s - intervals[p].first;
                //std::cout << "startWalksbyApp:\t" << "source" << i <<"=" << s << "\tp=" << p << "\tcur=" << cur << std::endl;
                WalkDataType walk = walk_manager.encode(s,cur,0);
                walk_manager.walks[p].push_back(walk);
                walk_manager.minstep[p] = 0;
            }
            walk_manager.freshIntervalWalks();
        }

        void updateInfo(vid_t dstId, unsigned srcId){
            cnt_all++;
            //std::cout<<"updateInfo:\t"<<"dstId="<<dstId<<"\tsrcId="<<srcId<<std::endl;
            if (dstId == (vid_t)srcId){
                cnt_ok++;
            }
        }

        float computeResult(){
            float triangle_ratio = (float) cnt_ok / (float) cnt_all;
            return triangle_ratio;
        }
        unsigned getCntAll(){
            return cnt_all;
        }
        unsigned getCntOK(){
            return cnt_ok;
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

    int nshards = convert_if_notexists(filename, get_option_string("nshards", "auto")); //detect the number of shards or preprocess an input to create them.

    // run
    graphLet program;
    program.initializeApp(N,R,L,tail);
    graphwalker_engine engine(filename, nshards, m);
    engine.run(program, prob);

    float triangle_ratio = program.computeResult();
    std::cout << "The ratio of triangle:\t" << triangle_ratio << std::endl;
    std::cout << "cnt_all:\t" << program.getCntAll() << std::endl;
    std::cout << "cnt_ok:\t" << program.getCntOK() << std::endl;

    metrics_report(m);
    return 0;
}