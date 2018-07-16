#ifndef RANDOMWALKWITHSUNK
#define RANDOMWALKWITHSUNK

#include <string>
#include <fstream>
#include <time.h>

#include "walks/walk.hpp"
#include "api/datatype.hpp"

class RandomWalkwithSunk : public RandomWalk {
    void updateByWalk(WalkDataType walk, int exec_interval, std::vector<Vertex> &vertices, WalkManager &walk_manager){
        WalkDataType nowwalk = walk;
        vid_t curId = walk_manager.getCurrentId(nowwalk) + intervals[exec_interval].first;
        vid_t dstId = curId;
        int hop = walk_manager.getHop(nowwalk);
        //std::cout << "updateByWalk:\t" << "dstId=" << dstId << "\thop=" << hop << "\tnsteps=" << nsteps << "\texec_interval=" << exec_interval <<  std::endl;
        while (dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second && hop%nsteps != nsteps-1){
            Vertex &nowVertex = vertices[dstId - intervals[exec_interval].first];

            if (nowVertex.outd > 0)
                dstId = random_outneighbor(nowVertex);
            else
                dstId = (vid_t)-1;  // sunk
            
            hop ++;
            nowwalk++;
        }

        if (dstId == (vid_t)-1){ // sunk
            ;
        }
        else if (hop%nsteps != nsteps -1){ // not finished, continue
            int p = getInterval(dstId);
            walk_manager.moveWalk(nowwalk, p, dstId - intervals[p].first);
            walk_manager.setMinStep(p, hop);
        }
        else{ // finished 
            updateInfo(dstId , walk_manager.getSourceId(walk));
        }
    }
};

#endif