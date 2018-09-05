/**
 *  for SimRank
 */
#ifndef RANDOMWALKWITHSUNK
#define RANDOMWALKWITHSUNK

#include <string>
#include <fstream>
#include <time.h>

#include "walks/walk.hpp"
#include "api/datatype.hpp"

class RandomWalkwithSunk : public RandomWalk {
    /**
     *  Walk update function.
     */
    void updateByWalk(WalkDataType walk, unsigned walkid, unsigned exec_interval, Vertex *&vertices, WalkManager &walk_manager ){ 
        //get current time in microsecond as seed to compute rand_r
        unsigned threadid = omp_get_thread_num();
        WalkDataType nowwalk = walk;
        vid_t curId = walk_manager.getCurrentId(nowwalk) + intervals[exec_interval].first;
        vid_t dstId = curId;
        unsigned hop = walk_manager.getHop(nowwalk);
        // unsigned seed = (unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count();
        unsigned seed = walk+curId+hop+(unsigned)time(NULL);
        while (dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second && hop < nsteps ){
            updateInfo(walk_manager, nowwalk, dstId);
            Vertex &nowVertex = vertices[dstId - intervals[exec_interval].first];
            if (nowVertex.outd > 0 && ((float)rand_r(&seed))/RAND_MAX > 0.15){
                dstId = random_outneighbor(nowVertex, seed);
            }else{
                dstId = 0xffffffff;
                hop = nsteps;
                break;
            }
            hop++;
            nowwalk++;
        }
        if( hop < nsteps ){
            unsigned p = getInterval( dstId );
            walk_manager.moveWalk(nowwalk, p, threadid, dstId - intervals[p].first);
            walk_manager.setMinStep( p, hop );
        }
    }
};

#endif