#ifndef RANDOMWALKWITHJUMP
#define RANDOMWALKWITHJUMP

#include <string>
#include <fstream>
#include <time.h>

#include "walks/walk.hpp" 
#include "api/datatype.hpp"

/**
 * Type definitions. Remember to create suitable graph shards using the
 * Sharder-program.
 */
 
class RandomWalkwithJump : public RandomWalk{
    /**
     *  Walk update function.
     */
    void updateByWalk(WalkDataType walk, int exec_interval, Vertex *&vertices, WalkManager &walk_manager){
            // std::cout << "updateByWalk ... " << std::endl;
            static unsigned t = (unsigned)time(NULL);
            static unsigned int seed[4] = {t,t*2,t*3,t*4};
            unsigned thread = omp_get_thread_num();
            WalkDataType nowwalk = walk;
            vid_t curId = walk_manager.getCurrentId(nowwalk) + intervals[exec_interval].first;
            vid_t dstId = curId;
            int hop = walk_manager.getHop(nowwalk);
            while (dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second && hop < nsteps ){
                Vertex &nowVertex = vertices[dstId - intervals[exec_interval].first];
                // updateInfo(walk_manager, nowwalk, dstId);
                updateInfo(dstId);
                if (nowVertex.outd > 0 && ((float)rand_r(&seed[thread]))/RAND_MAX > 0.15){
                    dstId = random_outneighbor(nowVertex,seed[thread]);
                }
                else{
                    dstId = rand_r(&seed[thread]) % nvertices;
                }
                hop++;
                nowwalk++;
            }
            if( hop < nsteps ){
                int p = getInterval( dstId );
                if(p==-1) logstream(LOG_FATAL) << "Invalid p = -1 with dstId = " << dstId << std::endl;
                walk_manager.moveWalk(nowwalk, p, thread, dstId - intervals[p].first);
                walk_manager.setMinStep( p, hop );
            }
            // else if(dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second){
            //     updateInfo(dstId, vertices[dstId - intervals[exec_interval].first].outd);
            // }
    }
};

#endif