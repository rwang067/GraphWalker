#ifndef RANDOMWALKWITHRESTART
#define RANDOMWALKWITHRESTART

#include <string>
#include <fstream>
#include <time.h>

#include "walks/walk.hpp" 
#include "api/datatype.hpp"

/**
 * Type definitions. Remember to create suitable graph shards using the
 * Sharder-program.
 */
 
class RandomWalkwithRestart : public RandomWalk {
public:
    vid_t source;

public:
    void initializeRW( vid_t _source, int _nwalks, int _nsteps, float _tail) {
        source = _source;
        nwalks = _nwalks;
        nsteps = _nsteps;
        tail = _tail;
        tailwalknum = (int)(nwalks*tail);
        std::cout << "nwalks, nsteps, tail, tailwalknum : "  << nwalks << " " << nsteps << " " << tail << " " << tailwalknum << std::endl;
    }
    /**
     *  Walk update function.
     */
    //void updateByWalk(std::vector<graphchi_vertex<VertexDataType, EdgeDataType> > &vertices, vid_t vid, int sub_interval_st, int sub_interval_en, walkManager &walk_manager, graphchi_context &gcontext){
    void updateByWalk(WalkDataType walk, int exec_interval, std::vector<Vertex > &vertices, WalkManager &walk_manager ){ //, VertexDataType* vertex_value){
            WalkDataType nowWalk = walk;
            vid_t dstId = walk_manager.getCurrentId(nowWalk) + intervals[exec_interval].first;
            int hop = walk_manager.getHop(nowWalk);
            while (dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second && hop < nsteps ){
                // std::cout  << " -> " << dstId ;
                if( hop == nsteps-1 )
                    updateInfo(dstId);
                Vertex &nowVertex = vertices[dstId - intervals[exec_interval].first];
                //nowVertex.set_data(nowVertex.get_data()+1);
                float cc = ((float)rand())/RAND_MAX;
                if (nowVertex.outd > 0 && cc > 0.15 )
                    dstId = random_outneighbor(nowVertex);
                else
                    dstId = walk_manager.getSourceId(walk) + source;
                hop++;
                nowWalk++;
            }
            // std::cout  << " hop, to " << hop << " " << dstId << std::endl;
            if( hop < nsteps ){
                int p = getInterval( dstId );
                walk_manager.moveWalk(nowWalk, p, dstId - intervals[p].first);
                walk_manager.setMinStep( p, hop );
            }else if(dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second) {
                ;//updateInfo(dstId);
            }
            // std::cout << " move walk " << nowWalk << "  " << dstId << "  " << hop << std::endl;
    }   

};

#endif