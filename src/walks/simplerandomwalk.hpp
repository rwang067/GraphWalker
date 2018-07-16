#ifndef SIMPLERANDOMWALK
#define SIMPLERANDOMWALK

#include <string>
#include <fstream>
#include <time.h>

#include "walks/walk.hpp" 
#include "api/datatype.hpp"

/**
 * Type definitions. Remember to create suitable graph shards using the
 * Sharder-program.
 */
 
class SimpleRandomWalk : public RandomWalk{
    /**
     *  Walk update function.
     */
    void updateByWalk(WalkDataType walk, int exec_interval, std::vector<Vertex > &vertices, WalkManager &walk_manager){
            // std::cout << "updateByWalk ... " << std::endl;
            WalkDataType nowwalk = walk;
            vid_t curId = walk_manager.getCurrentId(nowwalk) + intervals[exec_interval].first;
            vid_t dstId = curId;
            int hop = walk_manager.getHop(nowwalk);
            while (dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second && hop%nsteps != nsteps-1 ){
                // std::cout  << " -> " << dstId ;
                Vertex &nowVertex = vertices[dstId - intervals[exec_interval].first];
                updateInfo(walk_manager, nowwalk, dstId);
                updateInfo(dstId);
                //nowVertex.set_data(nowVertex.get_data()+1);
                if (nowVertex.outd > 0)
                    dstId = random_outneighbor(nowVertex);
                else
                    dstId = rand() % nvertices;
                hop++;
                nowwalk++;
            }
            // std::cout  << " to " << dstId << std::endl;
            if( hop%nsteps != nsteps-1 ){
                int p = getInterval( dstId );
                walk_manager.moveWalk(nowwalk, p, dstId - intervals[p].first);
                walk_manager.setMinStep( p, hop );
            }else if(dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second){
                updateInfo(dstId, vertices[dstId - intervals[exec_interval].first].outd);
            }
            // std::cout << " move walk " << nowwalk << "  " << dstId << "  " << hop << std::endl;
    }
};

#endif