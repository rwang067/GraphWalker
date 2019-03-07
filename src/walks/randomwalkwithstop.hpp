#ifndef RANDOMWALKWITHSTOP
#define RANDOMWALKWITHSTOP

#include <string>
#include <fstream>
#include <time.h>

#include "walks/walk.hpp" 
#include "api/datatype.hpp"

/**
 * Type definitions. Remember to create suitable graph shards using the
 * Sharder-program.
 */
 
class RandomWalkwithStop : public RandomWalk {

    unsigned numsources, walkspersource, maxwalklength;

public:
    void initializeRW(unsigned _nwalks, unsigned _maxwalklength) {
        nwalks = _nwalks;
        maxwalklength = _maxwalklength;
        tail = 0;
        tailwalknum = (unsigned)(nwalks*tail);
        std::cout << "initializeRW -- nwalks, maxwalklength : "  << nwalks << " " << maxwalklength << std::endl;
    }
    /**
     *  Walk update function.
     */
    //void updateByWalk(std::vector<graphchi_vertex<VertexDataType, EdgeDataType> > &vertices, vid_t vid, unsigned sub_interval_st, unsigned sub_interval_en, walkManager &walk_manager, graphchi_context &gcontext){
    void updateByWalk(WalkDataType walk, unsigned walkid, unsigned exec_interval, Vertex *&vertices, WalkManager &walk_manager ){ //, VertexDataType* vertex_value){
            unsigned threadid = omp_get_thread_num();
            WalkDataType nowWalk = walk;
            vid_t sourId = walk_manager.getSourceId(nowWalk);
            vid_t dstId = walk_manager.getCurrentId(nowWalk) + intervals[exec_interval].first;
            unsigned hop = walk_manager.getHop(nowWalk);
            unsigned seed = walkid+dstId+hop+(unsigned)time(NULL);
            while (dstId >= intervals[exec_interval].first && dstId <= intervals[exec_interval].second && hop < maxwalklength ){
                // std::cout  << " -> " << dstId << " " << walk_manager.getSourceId(walk) << std::endl;
                updateInfo(sourId, dstId, threadid, hop);
                Vertex &nowVertex = vertices[dstId - intervals[exec_interval].first];
                if (nowVertex.outd > 0 && ((float)rand_r(&seed))/RAND_MAX > 0.15 )
                    dstId = random_outneighbor(nowVertex, seed);
                else
                    return ;
                hop++;
                nowWalk++;
            }
            if( hop < maxwalklength ){
                unsigned p = getInterval( dstId );
                walk_manager.moveWalk(nowWalk, p, threadid, dstId - intervals[p].first);
                walk_manager.setMinStep( p, hop );
            }
    }   

};

#endif