#ifndef RANDOMWALKWITHJUMP
#define RANDOMWALKWITHJUMP

#include <string>
#include <fstream>
#include <time.h>
#include <chrono>

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
            //get current time in microsecond as seed to compute rand_r
            unsigned seed = (unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count();
            WalkDataType nowwalk = walk;
            vid_t curId = walk_manager.getCurrentId(nowwalk) + intervals[exec_interval].first;
            vid_t dstId = curId;
            int hop = walk_manager.getHop(nowwalk);
            while (dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second && hop < nsteps ){
                Vertex &nowVertex = vertices[dstId - intervals[exec_interval].first];
                // updateInfo(walk_manager, nowwalk, dstId);
                updateInfo(dstId);
                if (nowVertex.outd > 0 && ((float)rand_r(&seed))/RAND_MAX > 0.15){
                    dstId = random_outneighbor(nowVertex,seed);
                }
                else{
                    dstId = rand_r(&seed) % nvertices;
                }
                hop++;
                nowwalk++;
            }
            if( hop < nsteps ){
                int p = getInterval( dstId );
                if(p==-1) logstream(LOG_FATAL) << "Invalid p = -1 with dstId = " << dstId << std::endl;
                walk_manager.moveWalk(nowwalk, p, omp_get_thread_num(), dstId - intervals[p].first);
                walk_manager.setMinStep( p, hop );
            }
            // else if(dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second){
            //     updateInfo(dstId, vertices[dstId - intervals[exec_interval].first].outd);
            // }
    }
};

#endif